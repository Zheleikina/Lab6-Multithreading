cmake_minimum_required(VERSION 3.20)
project(Lab6_Multithreading CXX)

# Встановлюємо стандарт C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Додавання багатопотоковості для сумісності з різними ОС (для Windows/MSVC це вбудовано)
find_package(Threads REQUIRED)

# Список ваших виконуваних файлів (завдань)
add_executable(race_no_sync src/race_no_sync.cpp)
add_executable(race_mutex src/race_mutex.cpp)
add_executable(race_atomic src/race_atomic.cpp)

add_executable(deadlock_hang src/deadlock_hang.cpp)
add_executable(deadlock_scoped_lock src/deadlock_scoped_lock.cpp)
add_executable(deadlock_ordered src/deadlock_ordered.cpp)

add_executable(image_pipeline src/image_pipeline.cpp)
add_executable(primes_async src/primes_async.cpp)

# Підключення потоків до таргетів за потреби
target_link_libraries(primes_async PRIVATE Threads::Threads)
target_link_libraries(image_pipeline PRIVATE Threads::Threads)