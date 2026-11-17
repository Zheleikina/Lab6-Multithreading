// Частина 3, варіант 3: конвеєр обробки зображень (спрощено).
// Виробник генерує завдання (ім'я файлу-заглушки), споживачі виконують фіктивну обробку (затримка).
//
// Параметри командного рядка (усі необов'язкові), потрібні для завдань самостійного аналізу:
//   image_pipeline.exe [consumers=2] [msPerComplexity=150] [useNotify=1]
// Приклади:
//   image_pipeline.exe             -> 2 споживачі (базовий запуск)
//   image_pipeline.exe 1           -> 1 споживач
//   image_pipeline.exe 4           -> 4 споживачі
//   image_pipeline.exe 2 400       -> збільшена затримка обробки (завдання 3)
//   image_pipeline.exe 2 150 0     -> без notify_one() (завдання 4)
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <string>
#include <chrono>
#include <vector>
#include <cstdio>

struct ImageTask
{
    int id;
    std::string fileName;   // ім'я файлу-заглушки
    std::string operation;  // тип обробки
    int complexity;         // 1..3
};

std::queue<ImageTask> tasks;
std::mutex queueMutex;               // захист черги, finished, maxQueueSize
std::mutex outputMutex;              // захист виводу та лічильників обробки
std::condition_variable condition;
bool finished = false;
size_t maxQueueSize = 0;

const int TASK_COUNT = 12;
std::vector<int> processedCount(TASK_COUNT + 1, 0); // скільки разів оброблено кожне id

// Налаштування експерименту (змінюються аргументами командного рядка)
int consumerCount = 2;
int msPerComplexity = 150;
bool useNotify = true;

void producer()
{
    const std::string operations[] = { "grayscale", "blur", "resize", "sharpen" };

    for (int i = 1; i <= TASK_COUNT; ++i)
    {
        char name[32];
        std::snprintf(name, sizeof(name), "image_%03d.jpg", i);

        ImageTask task{ i, name, operations[(i - 1) % 4], i % 3 + 1 };

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            tasks.push(task);
            if (tasks.size() > maxQueueSize) maxQueueSize = tasks.size();
        }
        {
            std::lock_guard<std::mutex> lock(outputMutex);
            std::cout << "[Producer] Created task " << task.id
                      << ", file: " << task.fileName
                      << ", operation: " << task.operation << '\n';
        }

        if (useNotify)
            condition.notify_one(); // будимо одного сплячого споживача

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        finished = true; // ознака завершення змінюється під м'ютексом
    }
    condition.notify_all(); // будимо всіх: вони обробять залишок черги й завершаться

    {
        std::lock_guard<std::mutex> lock(outputMutex);
        std::cout << "[Producer] Generation finished\n";
    }
}

void consumer(int consumerId)
{
    while (true)
    {
        ImageTask task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, []() { return !tasks.empty() || finished; });

            if (tasks.empty() && finished)
                break;

            task = tasks.front();
            tasks.pop();
        }

        // Обробка -- ПОЗА критичною секцією
        std::this_thread::sleep_for(
            std::chrono::milliseconds(task.complexity * msPerComplexity));

        {
            std::lock_guard<std::mutex> lock(outputMutex);
            ++processedCount[task.id];
            std::cout << "[Consumer " << consumerId << "] Processed task " << task.id
                      << ", file: " << task.fileName
                      << ", operation: " << task.operation
                      << ", complexity: " << task.complexity << '\n';
        }
    }

    std::lock_guard<std::mutex> lock(outputMutex);
    std::cout << "[Consumer " << consumerId << "] Finished\n";
}

int main(int argc, char* argv[])
{
    if (argc > 1) consumerCount = std::stoi(argv[1]);
    if (argc > 2) msPerComplexity = std::stoi(argv[2]);
    if (argc > 3) useNotify = (std::stoi(argv[3]) != 0);

    std::cout << "Consumers: " << consumerCount
              << ", ms per complexity unit: " << msPerComplexity
              << ", notify_one after push: " << (useNotify ? "yes" : "NO") << "\n\n";

    auto start = std::chrono::steady_clock::now();

    std::thread producerThread(producer);
    std::vector<std::thread> consumers;
    for (int i = 1; i <= consumerCount; ++i)
        consumers.emplace_back(consumer, i);

    producerThread.join();
    for (auto& c : consumers)
        c.join();

    auto end = std::chrono::steady_clock::now();
    double totalMs = std::chrono::duration<double, std::milli>(end - start).count();

    // Перевірка: кожна заявка оброблена рівно один раз
    bool allOk = true;
    for (int id = 1; id <= TASK_COUNT; ++id)
        if (processedCount[id] != 1) allOk = false;

    std::cout << "\nAll tasks processed exactly once: " << (allOk ? "YES" : "NO") << '\n';
    std::cout << "Max queue size: " << maxQueueSize << '\n';
    std::cout << "Total time: " << totalMs << " ms\n";
    return 0;
}
