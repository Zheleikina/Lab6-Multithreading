// Частина 4, завдання підвищеної складності №3: паралельний пошук простих чисел (std::async + std::future).
// Рахуємо кількість простих у діапазоні [2, N]. Діапазон ділиться на k рівних частин.
// Результати виводяться в консоль, записуються у results.csv та в SVG-графіки (відкрити в браузері).
//
// Запуск: primes_async.exe [repeats=5]   (збирати у Release x64, C++20)
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <future>
#include <thread>
#include <chrono>
#include <algorithm>
#include <string>
#include <sstream>

// ---------- Побудова графіків у форматі SVG (тільки стандартна бібліотека) ----------
struct Series
{
    std::string name;
    std::vector<double> y;
};

// Лінійний графік: по осі X -- підписи (кількість задач), по осі Y -- значення серій
void writeLineChartSvg(const std::string& fileName, const std::string& title,
                       const std::string& xLabel, const std::string& yLabel,
                       const std::vector<std::string>& xLabels,
                       const std::vector<Series>& series)
{
    const int W = 820, H = 520, L = 80, R = 40, T = 60, B = 80;
    const char* colors[] = { "#1f77b4", "#d62728", "#2ca02c", "#ff7f0e", "#9467bd", "#8c564b", "#e377c2", "#000000" };

    double yMax = 0;
    for (const auto& s : series)
        for (double v : s.y) yMax = std::max(yMax, v);
    if (yMax <= 0) yMax = 1;
    yMax *= 1.1;

    const double plotW = W - L - R, plotH = H - T - B;
    const size_t n = xLabels.size();
    auto xPos = [&](size_t i) { return L + (n > 1 ? plotW * i / (n - 1) : plotW / 2); };
    auto yPos = [&](double v) { return T + plotH * (1.0 - v / yMax); };

    std::ofstream o(fileName);
    o << std::fixed << std::setprecision(2);
    o << "<svg xmlns='http://www.w3.org/2000/svg' width='" << W << "' height='" << H
      << "' viewBox='0 0 " << W << ' ' << H << "' font-family='Arial' font-size='13'>\n";
    o << "<rect width='100%' height='100%' fill='white'/>\n";
    o << "<text x='" << W / 2 << "' y='30' text-anchor='middle' font-size='18' font-weight='bold'>"
      << title << "</text>\n";

    // горизонтальна сітка та підписи осі Y
    for (int k = 0; k <= 5; ++k)
    {
        double v = yMax * k / 5.0;
        o << "<line x1='" << L << "' y1='" << yPos(v) << "' x2='" << W - R << "' y2='" << yPos(v)
          << "' stroke='#dddddd'/>\n";
        o << "<text x='" << L - 8 << "' y='" << yPos(v) + 4 << "' text-anchor='end'>" << v << "</text>\n";
    }
    // осі
    o << "<line x1='" << L << "' y1='" << T << "' x2='" << L << "' y2='" << H - B << "' stroke='black'/>\n";
    o << "<line x1='" << L << "' y1='" << H - B << "' x2='" << W - R << "' y2='" << H - B << "' stroke='black'/>\n";
    // підписи осі X
    for (size_t i = 0; i < n; ++i)
        o << "<text x='" << xPos(i) << "' y='" << H - B + 20 << "' text-anchor='middle'>" << xLabels[i] << "</text>\n";
    o << "<text x='" << L + plotW / 2 << "' y='" << H - B + 45 << "' text-anchor='middle'>" << xLabel << "</text>\n";
    o << "<text transform='translate(22," << T + plotH / 2 << ") rotate(-90)' text-anchor='middle'>" << yLabel << "</text>\n";

    // лінії серій
    for (size_t si = 0; si < series.size(); ++si)
    {
        const char* c = colors[si % 8];
        o << "<polyline fill='none' stroke='" << c << "' stroke-width='2' points='";
        for (size_t i = 0; i < series[si].y.size(); ++i)
            o << xPos(i) << ',' << yPos(series[si].y[i]) << ' ';
        o << "'/>\n";
        for (size_t i = 0; i < series[si].y.size(); ++i)
            o << "<circle cx='" << xPos(i) << "' cy='" << yPos(series[si].y[i]) << "' r='3.5' fill='" << c << "'/>\n";
        // легенда
        double ly = T + 8 + 20 * si;
        o << "<rect x='" << W - R - 190 << "' y='" << ly - 9 << "' width='14' height='14' fill='" << c << "'/>\n";
        o << "<text x='" << W - R - 170 << "' y='" << ly + 3 << "'>" << series[si].name << "</text>\n";
    }
    o << "</svg>\n";
}

// Перевірка простоти перебором дільників виду 6k±1
bool isPrime(int n)
{
    if (n < 2) return false;
    if (n < 4) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (long long i = 5; i * i <= n; i += 6)
        if (n % i == 0 || n % (i + 2) == 0)
            return false;
    return true;
}

// Послідовна версія: кількість простих у [lo, hi]
long long countPrimesSequential(int lo, int hi)
{
    long long count = 0;
    for (int n = lo; n <= hi; ++n)
        if (isPrime(n)) ++count;
    return count;
}

// Паралельна версія: діапазон -> tasks незалежних під-діапазонів, кожен у std::async
long long countPrimesParallel(int lo, int hi, int tasks)
{
    const long long total = static_cast<long long>(hi) - lo + 1;
    const long long chunk = total / tasks;
    const long long rem = total % tasks;

    std::vector<std::future<long long>> futures;
    futures.reserve(tasks);

    long long start = lo;
    for (int t = 0; t < tasks; ++t)
    {
        long long size = chunk + (t < rem ? 1 : 0);
        if (size == 0) continue;
        long long end = start + size - 1;

        futures.push_back(std::async(std::launch::async, countPrimesSequential,
                                     static_cast<int>(start), static_cast<int>(end)));
        start = end + 1;
    }

    long long result = 0;
    for (auto& f : futures)
        result += f.get(); // об'єднання часткових результатів
    return result;
}

// Вимірює СЕРЕДНІЙ час (мс) серії запусків функції; result -- результат останнього запуску
template <typename F>
double measureMeanMs(F func, int repeats, long long& result)
{
    double sum = 0;
    for (int r = 0; r < repeats; ++r)
    {
        auto start = std::chrono::steady_clock::now();
        result = func();
        auto end = std::chrono::steady_clock::now();
        sum += std::chrono::duration<double, std::milli>(end - start).count();
    }
    return sum / repeats;
}

int main(int argc, char* argv[])
{
    const int repeats = (argc > 1) ? std::stoi(argv[1]) : 5;
    const unsigned hw = std::max(1u, std::thread::hardware_concurrency());

    const std::vector<int> sizes = { 1'000'000, 5'000'000, 10'000'000 }; // 3 розміри діапазону [2, N]
    const std::vector<int> taskCounts = { 1, 2, 4, 8, 16, 32, 64 };

    std::cout << "Hardware threads: " << hw << ", repeats (mean): " << repeats << "\n\n";

    std::ofstream csv("results.csv");
    csv << "range_max,tasks,seq_ms,par_ms,speedup,efficiency,primes,correct\n";

    std::cout << std::left
              << std::setw(12) << "N" << std::setw(8) << "Tasks"
              << std::setw(12) << "Seq, ms" << std::setw(12) << "Par, ms"
              << std::setw(10) << "Speedup" << std::setw(12) << "Efficiency"
              << std::setw(10) << "Primes" << "Correct\n";

    std::vector<std::string> xLabels;
    for (int t : taskCounts) xLabels.push_back(std::to_string(t));
    std::vector<Series> speedupSeries, efficiencySeries, timeSeries;
    // для графіків від розміру даних: одна серія на кожне значення p
    std::vector<Series> timeBySize, speedupBySize;
    for (int t : taskCounts)
    {
        timeBySize.push_back({ "p = " + std::to_string(t), {} });
        speedupBySize.push_back({ "p = " + std::to_string(t), {} });
    }
    Series seqBySize{ "sequential", {} };
    std::vector<std::string> sizeLabels;

    for (int n : sizes)
    {
        const std::string label = "N = " + std::to_string(n);
        Series sp{ label, {} }, ef{ label, {} }, tm{ label, {} };
        sizeLabels.push_back(std::to_string(n));
        long long seqResult = 0;
        double seqMs = measureMeanMs([&] { return countPrimesSequential(2, n); }, repeats, seqResult);

        seqBySize.y.push_back(seqMs);
        size_t ti = 0;
        for (int tasks : taskCounts)
        {
            long long parResult = 0;
            double parMs = measureMeanMs([&] { return countPrimesParallel(2, n, tasks); }, repeats, parResult);

            bool correct = (seqResult == parResult);
            double speedup = seqMs / parMs;
            double efficiency = speedup / tasks; // E = S / p, p -- кількість задач в експерименті

            std::cout << std::left << std::fixed << std::setprecision(2)
                      << std::setw(12) << n << std::setw(8) << tasks
                      << std::setw(12) << seqMs << std::setw(12) << parMs
                      << std::setw(10) << speedup << std::setw(12) << efficiency
                      << std::setw(10) << parResult << (correct ? "YES" : "NO") << '\n';

            timeBySize[ti].y.push_back(parMs);
            speedupBySize[ti].y.push_back(speedup);
            ++ti;
            sp.y.push_back(speedup);
            ef.y.push_back(efficiency);
            tm.y.push_back(parMs);

            csv << n << ',' << tasks << ',' << seqMs << ',' << parMs << ','
                << speedup << ',' << efficiency << ',' << parResult << ',' << (correct ? 1 : 0) << '\n';
        }
        speedupSeries.push_back(sp);
        efficiencySeries.push_back(ef);
        timeSeries.push_back(tm);
        std::cout << '\n';
    }

    writeLineChartSvg("speedup_vs_tasks.svg", "Speedup vs number of async tasks",
                      "Number of tasks", "Speedup S", xLabels, speedupSeries);
    writeLineChartSvg("efficiency_vs_tasks.svg", "Efficiency vs number of async tasks",
                      "Number of tasks", "Efficiency E", xLabels, efficiencySeries);
    writeLineChartSvg("time_vs_tasks.svg", "Parallel execution time vs number of async tasks",
                      "Number of tasks", "Time, ms", xLabels, timeSeries);

    timeBySize.push_back(seqBySize);
    writeLineChartSvg("time_vs_size.svg", "Execution time vs data size",
                      "Range upper bound N", "Time, ms", sizeLabels, timeBySize);
    writeLineChartSvg("speedup_vs_size.svg", "Speedup vs data size",
                      "Range upper bound N", "Speedup S", sizeLabels, speedupBySize);

    std::cout << "Results saved to results.csv\n";
    std::cout << "Charts saved: speedup_vs_tasks, efficiency_vs_tasks, time_vs_tasks, "
                 "time_vs_size, speedup_vs_size (.svg)\n";
    return 0;
}
