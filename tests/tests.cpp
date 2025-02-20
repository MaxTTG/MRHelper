#include <any>
#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "MRHelper/MapReduceTask.hpp"
#include "MRHelper/Scheduler.hpp"
#include "MRHelper/SimpleTask.hpp"
#include "MRHelper/Task.hpp"
#include "MRHelper/ThreadPool.hpp"

void testSimpleTaskCaching() {
    std::cout << "Running testSimpleTaskCaching..." << std::endl;
    int executionCount = 0;
    auto task          = std::make_shared<mrh::SimpleTask>(
        [&executionCount](const std::vector<std::any>& inputs) -> std::any {
            ++executionCount;
            return std::string("cached_result");
        },
        true);

    mrh::ThreadPool pool(2);
    std::any result1 = task->execute(pool);
    std::any result2 = task->execute(pool);

    assert(std::any_cast<std::string>(result1) == "cached_result");
    assert(std::any_cast<std::string>(result2) == "cached_result");
    assert(executionCount == 1);
    std::cout << "testSimpleTaskCaching passed." << std::endl;
}

void testSimpleTaskNoCaching() {
    std::cout << "Running testSimpleTaskNoCaching..." << std::endl;
    int executionCount = 0;
    auto task          = std::make_shared<mrh::SimpleTask>(
        [&executionCount](const std::vector<std::any>& inputs) -> std::any {
            ++executionCount;
            return executionCount;
        },
        false);

    mrh::ThreadPool pool(2);
    std::any result1 = task->execute(pool);
    std::any result2 = task->execute(pool);

    int r1 = std::any_cast<int>(result1);
    int r2 = std::any_cast<int>(result2);
    assert(r1 == 1);
    assert(r2 == 2);
    std::cout << "testSimpleTaskNoCaching passed." << std::endl;
}

void testMapReduceTask() {
    std::cout << "Running testMapReduceTask..." << std::endl;
    auto mapFunc = [](const std::vector<int>& input) -> std::vector<std::pair<std::string, int>> {
        return {{"a", 1}, {"b", 2}, {"a", 3}};
    };
    auto reduceFunc = [](const std::string& key, const std::vector<int>& values) -> int {
        int sum = 0;
        for (int v : values)
            sum += v;
        return sum;
    };
    auto task =
        std::make_shared<mrh::MapReduceTask<std::vector<int>, std::string, int, int>>(mapFunc, reduceFunc, 1, true);

    mrh::ThreadPool pool(2);
    std::any result = task->execute(pool);
    auto resMap     = std::any_cast<std::map<std::string, int>>(result);
    assert(resMap["a"] == 4);
    assert(resMap["b"] == 2);
    std::cout << "testMapReduceTask passed." << std::endl;
}

void testSchedulerIntegration() {
    std::cout << "Running testSchedulerIntegration..." << std::endl;
    auto task1 = std::make_shared<mrh::SimpleTask>(
        [](const std::vector<std::any>& inputs) -> std::any {
            return std::vector<int>{1, 2, 3, 4, 5};
        },
        true);
    auto task2 = std::make_shared<mrh::SimpleTask>(
        [](const std::vector<std::any>& inputs) -> std::any {
            auto vec = std::any_cast<std::vector<int>>(inputs[0]);
            std::reverse(vec.begin(), vec.end());
            return vec;
        },
        true);
    auto task3 = std::make_shared<mrh::SimpleTask>(
        [](const std::vector<std::any>& inputs) -> std::any {
            auto vec = std::any_cast<std::vector<int>>(inputs[0]);
            int sum  = 0;
            for (int v : vec)
                sum += v;
            return sum;
        },
        true);
    task2->dependsOn(task1);
    task3->dependsOn(task2);

    mrh::Scheduler scheduler(3);
    auto future     = scheduler.submit(task3);
    std::any result = future.get();
    int sum         = std::any_cast<int>(result);
    assert(sum == 15);
    std::cout << "testSchedulerIntegration passed." << std::endl;
}

void testSchedulerOneThread() {
    std::cout << "Running testSchedulerOneThread..." << std::endl;
    auto task1 = std::make_shared<mrh::SimpleTask>(
        [](const std::vector<std::any>& inputs) -> std::any { return std::string("one-thread"); }, true);
    auto task2 = std::make_shared<mrh::SimpleTask>(
        [](const std::vector<std::any>& inputs) -> std::any {
            auto s = std::any_cast<std::string>(inputs[0]);
            return s + " executed";
        },
        true);
    task2->dependsOn(task1);
    mrh::Scheduler scheduler(1);
    auto future        = scheduler.submit(task2);
    std::any result    = future.get();
    std::string resStr = std::any_cast<std::string>(result);
    assert(resStr == "one-thread executed");
    std::cout << "testSchedulerOneThread passed." << std::endl;
}

int main() {
    std::cout << "Running MRHelper tests..." << std::endl;
    testSimpleTaskCaching();
    testSimpleTaskNoCaching();
    testMapReduceTask();
    testSchedulerIntegration();
    testSchedulerOneThread();
    std::cout << "All tests passed." << std::endl;
    return 0;
}
