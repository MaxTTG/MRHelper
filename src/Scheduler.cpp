#include "MRHelper/Scheduler.hpp"

#include <future>
#include <stdexcept>
#include <thread>
#include <vector>

namespace mrh {

Scheduler::Scheduler(size_t numThreads) : _threadPool(numThreads) {}

Scheduler::~Scheduler() {}

void Scheduler::buildGraph(const std::shared_ptr<Task>& task, std::unordered_map<Task*, int>& indegree,
                           std::unordered_map<Task*, std::vector<std::shared_ptr<Task>>>& graph,
                           std::unordered_map<Task*, std::shared_ptr<Task>>& taskMap,
                           std::unordered_map<Task*, bool>& visited) {
    if (!task || visited[task.get()])
        return;
    visited[task.get()]  = true;
    taskMap[task.get()]  = task;
    indegree[task.get()] = static_cast<int>(task->getDependencies().size());
    for (auto& dep : task->getDependencies()) {
        graph[dep.get()].push_back(task);
        buildGraph(dep, indegree, graph, taskMap, visited);
    }
}

std::any Scheduler::execute(const std::shared_ptr<Task>& root) {
    std::unordered_map<Task*, int> indegree;
    std::unordered_map<Task*, std::vector<std::shared_ptr<Task>>> graph;
    std::unordered_map<Task*, std::shared_ptr<Task>> taskMap;
    std::unordered_map<Task*, bool> visited;
    buildGraph(root, indegree, graph, taskMap, visited);

    std::unordered_map<Task*, std::atomic<int>> remainingDeps;
    for (auto& pair : indegree) {
        remainingDeps[pair.first] = pair.second;
    }

    std::mutex queueMutex;
    std::queue<std::shared_ptr<Task>> readyQueue;
    for (auto& pair : taskMap) {
        Task* t = pair.first;
        if (remainingDeps[t] == 0) {
            readyQueue.push(pair.second);
        }
    }

    std::atomic<int> tasksRemaining(static_cast<int>(taskMap.size()));

    while (tasksRemaining > 0) {
        std::shared_ptr<Task> task;
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (!readyQueue.empty()) {
                task = readyQueue.front();
                readyQueue.pop();
            }
        }
        if (task) {
            task->execute(_threadPool);
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                for (auto& dependent : graph[task.get()]) {
                    int newVal = --remainingDeps[dependent.get()];
                    if (newVal == 0) {
                        readyQueue.push(dependent);
                    }
                }
            }
            --tasksRemaining;
        } else {
            if (!_threadPool.tryExecuteOne()) {
                std::this_thread::yield();
            }
        }
    }

    return root->getResult();
}

std::future<std::any> Scheduler::submit(const std::shared_ptr<Task>& root) {
    return _threadPool.enqueue([this, root]() { return this->execute(root); });
}

}  // namespace mrh
