#include <algorithm>
#include <any>
#include <iostream>
#include <map>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include "MRHelper/MapReduceTask.hpp"
#include "MRHelper/Scheduler.hpp"
#include "MRHelper/SimpleTask.hpp"

std::mutex coutMutex;
#define MUTE std::lock_guard<std::mutex> lg(coutMutex)

void printVector(const std::vector<int>& vec) {
    MUTE;
    for (const auto& v : vec)
        std::cout << v << ' ';
    std::cout << std::endl;
}

int main() {
    // Создаем Scheduler с 2 потоками.
    mrh::Scheduler scheduler(2);

    // Пример 1: Простая задача, генерирующая случайный вектор чисел.
    auto task1 = std::make_shared<mrh::SimpleTask>([](const std::vector<std::any>& inputs) -> std::any {
        std::cout << "Executing Task1: Generating random vector..." << std::endl;
        std::vector<int> vec;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, 100);
        for (int i = 0; i < 10; ++i)
            vec.push_back(dist(gen));
        printVector(vec);
        return vec;
    });

    // Пример 2: Задача, которая переворачивает вектор, полученный из task1.
    auto task2 = std::make_shared<mrh::SimpleTask>([](const std::vector<std::any>& inputs) -> std::any {
        std::cout << "Executing Task2: Reversing vector..." << std::endl;
        auto vec = std::any_cast<std::vector<int>>(inputs[0]);
        std::reverse(vec.begin(), vec.end());
        printVector(vec);
        return vec;
    });
    task2->dependsOn(task1);

    // Пример 3: MapReduce-задача, которая выполняет простую агрегацию.
    // Здесь Input = std::vector<int>, Key = std::string, Value = int, Output = int.
    auto mapFunc = [](const std::vector<int>& input) -> std::vector<std::pair<std::string, int>> {
        MUTE;
        std::cout << "Executing Map in Task3..." << std::endl;
        return {{"sum", 10}, {"sum", 20}, {"sum", 30}};
    };
    auto reduceFunc = [](const std::string& key, const std::vector<int>& values) -> int {
        {
            MUTE;
            std::cout << "[thread " << std::this_thread::get_id() << "] Executing Reduce in Task3..." << std::endl;
        }
        int total = 0;
        for (int v : values)
            total += v;
        return total;
    };
    auto task3 = std::make_shared<mrh::MapReduceTask<std::vector<int>, std::string, int, int>>(mapFunc, reduceFunc, 1);
    task3->dependsOn(task2);

    // Выполнение всей цепочки через Scheduler.
    auto future     = scheduler.submit(task3);
    std::any result = future.get();
    auto resMap     = std::any_cast<std::map<std::string, int>>(result);

    std::cout << "MapReduce result:" << std::endl;
    for (const auto& kv : resMap) {
        std::cout << kv.first << " : " << kv.second << std::endl;
    }

    return 0;
}
