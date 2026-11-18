// Частина 1.3: виправлення через std::mutex + std::lock_guard.
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

constexpr int STUDENT_NUMBER = 3;
constexpr int ITERATIONS = STUDENT_NUMBER * 100000;
constexpr int RUNS = 5;

int counter = 0;
std::mutex counterMutex;

void increment()
{
    for (int i = 0; i < ITERATIONS; ++i)
    {
        std::lock_guard<std::mutex> lock(counterMutex); // RAII: звільнення при виході зі scope
        ++counter;
    }
}

int main()
{
    const int expected = 2 * ITERATIONS;
    int wrongRuns = 0;
    double totalMs = 0;

    std::cout << "=== std::mutex ===\n";
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

        bool ok = (counter == expected);
        if (!ok) ++wrongRuns;
        std::cout << "Run " << run << ": expected " << expected
                  << ", actual " << counter
                  << (ok ? " [OK]" : " [WRONG]")
                  << ", time " << ms << " ms\n";
    }
    std::cout << "Wrong runs: " << wrongRuns << " of " << RUNS << '\n';
    std::cout << "Average time: " << totalMs / RUNS << " ms\n";
    return 0;
}
