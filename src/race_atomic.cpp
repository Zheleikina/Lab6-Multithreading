// Частина 1.4: виправлення через std::atomic<int>.
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

constexpr int STUDENT_NUMBER = 1;                  // <-- ВСТАВТЕ СВІЙ НОМЕР ЗА СПИСКОМ
constexpr int ITERATIONS = STUDENT_NUMBER * 100000;
constexpr int RUNS = 5;

std::atomic<int> counter{0};

void increment()
{
    for (int i = 0; i < ITERATIONS; ++i)
    {
        ++counter; // атомарний інкремент
    }
}

int main()
{
    const int expected = 2 * ITERATIONS;
    int wrongRuns = 0;
    double totalMs = 0;

    std::cout << "=== std::atomic ===\n";
    for (int run = 1; run <= RUNS; ++run)
    {
        counter = 0;
        auto start = std::chrono::steady_clock::now();

        std::thread t1(increment);
        std::thread t2(increment);
        t1.join();
        t2.join();

        auto end = std::chrono::steady_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        totalMs += ms;

        int actual = counter.load();
        bool ok = (actual == expected);
        if (!ok) ++wrongRuns;
        std::cout << "Run " << run << ": expected " << expected
                  << ", actual " << actual
                  << (ok ? " [OK]" : " [WRONG]")
                  << ", time " << ms << " ms\n";
    }
    std::cout << "Wrong runs: " << wrongRuns << " of " << RUNS << '\n';
    std::cout << "Average time: " << totalMs / RUNS << " ms\n";
    return 0;
}
