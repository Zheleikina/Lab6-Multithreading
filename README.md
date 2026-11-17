# Лабораторна робота №6 — інструкція із запуску (Visual Studio 2022, C++20)

## Налаштування проєкту (один раз для кожного .cpp)
1. File → New → Project → **Console App (C++)**. Один проєкт = один .cpp (у кожному є свій `main`).
   Або в одному проєкті тримайте один активний файл, решту виключайте (Properties → Excluded From Build = Yes).
2. Project → Properties → C/C++ → Language → **C++ Language Standard = ISO C++20 (/std:c++20)**.
3. (Рекомендовано) C/C++ → Command Line → Additional Options: `/utf-8`.
4. Конфігурація:
   - Частини 1 (`race_no_sync`), 2, 3 — **Debug x64** (для гонки даних Release може оптимізувати цикл і сховати помилку).
   - Частина 1 (час mutex vs atomic) і Частина 4 — **Release x64** (Ctrl+F5, без відладчика).
5. У кожному файлі частини 1 змініть `STUDENT_NUMBER` на свій номер за списком.

## Частина 1 — `Part1_RaceCondition`
- `race_no_sync.cpp` — 10 запусків без синхронізації; виводить кількість невірних результатів.
- `race_mutex.cpp`, `race_atomic.cpp` — 5 запусків + середній час. Для порівняння часу запускайте обидва в Release x64.

## Частина 2 — `Part2_Deadlock`
- `deadlock_hang.cpp` — **має зависнути** (зробіть скріншот консолі, потім Shift+F5).
- `deadlock_scoped_lock.cpp`, `deadlock_ordered.cpp` — запустіть по 5+ разів, мають завершуватись.

## Частина 3 — `Part3_ProducerConsumer/image_pipeline.cpp` (варіант 3)
Аргументи: `[consumers] [msPerComplexity] [useNotify]`
(Properties → Debugging → Command Arguments, або через cmd у папці з exe):

| Завдання з методички | Аргументи |
|---|---|
| Базовий запуск (2 споживачі) | *(без аргументів)* |
| П.2: 1 споживач / 4 споживачі | `1` / `4` |
| П.3: збільшена затримка | `2 400` |
| П.4: без `notify_one()` | `2 150 0` |

Пп. 1, 5, 6 — перевірка у виводі («exactly once: YES») і текстові відповіді (є в звіті).

## Частина 4 — `Part4_PrimesAsync`
1. Release x64 → запустіть `primes_async.exe` (опційно `primes_async.exe 5` — кількість повторів, береться медіана). Закрийте важкі програми у фоні.
2. Програма створить `results.csv` у робочій папці (за замовчуванням — папка проєкту).
3. Графіки створюються автоматично (у папці з `results.csv`): `time_vs_size.svg`, `speedup_vs_size.svg`, `time_vs_tasks.svg`, `speedup_vs_tasks.svg`, `efficiency_vs_tasks.svg`. Відкрийте в браузері, зробіть скриншот або «Друк → PDF».
4. Таблиці для звіту беріть з консолі або `results.csv` (відкривається в Excel).
