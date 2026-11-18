// Частина 1.2: лічильник БЕЗ синхронізації (data race).
#include <iostream>
#include <thread>
#include <chrono>

constexpr int STUDENT_NUMBER = 3;
constexpr int ITERATIONS = STUDENT_NUMBER * 100000; // N * 100000
constexpr int RUNS = 10;                            // кількість запусків

int counter = 0;                                    // спільна змінна без захисту

void increment()
{
    for (int i = 0; i < ITERATIONS; ++i)
    {
        ++counter; // read-modify-write, не атомарно
    }
}

int main()
{
    const int expected = 2 * ITERATIONS;
    int wrongRuns = 0;

    std::cout << "=== Without synchronization ===\n";
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

        bool ok = (counter == expected);
        if (!ok) ++wrongRuns;

        std::cout << "Run " << run << ": expected " << expected
                  << ", actual " << counter
                  << (ok ? " [OK]" : " [WRONG]")
                  << ", time " << ms << " ms\n";
    }
    std::cout << "Wrong runs: " << wrongRuns << " of " << RUNS << '\n';
    return 0;
}
