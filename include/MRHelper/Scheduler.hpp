#pragma once

#include <any>
#include <future>
#include <memory>
#include <queue>
#include <unordered_map>

#include "MRHelper/Task.hpp"
#include "MRHelper/ThreadPool.hpp"

namespace mrh {

/**
 * @brief Scheduler for executing tasks with dependencies.
 *
 * Builds a dependency graph and executes tasks accordingly.
 */
class Scheduler {
public:
    /**
     * @brief Constructs a Scheduler.
     *
     * @param numThreads Number of threads in the pool.
     */
    Scheduler(size_t numThreads = std::thread::hardware_concurrency());

    /// Destructor.
    ~Scheduler();

    /**
     * @brief Executes the task graph starting from the root.
     *
     * @param root The root task.
     * @return The result of the root task as std::any.
     */
    std::any execute(const std::shared_ptr<Task>& root);

    /**
     * @brief Submits the root task for asynchronous execution.
     *
     * @param root The root task.
     * @return A future for the result.
     */
    std::future<std::any> submit(const std::shared_ptr<Task>& root);

private:
    ThreadPool _threadPool;  ///< Thread pool for executing tasks.

    /**
     * @brief Recursively builds the dependency graph.
     *
     * @param task Current task.
     * @param indegree Map of task indegrees.
     * @param graph Map from each task to its dependents.
     * @param taskMap Map from task pointers to shared_ptr.
     * @param visited Map to track visited tasks.
     */
    void buildGraph(const std::shared_ptr<Task>& task, std::unordered_map<Task*, int>& indegree,
                    std::unordered_map<Task*, std::vector<std::shared_ptr<Task>>>& graph,
                    std::unordered_map<Task*, std::shared_ptr<Task>>& taskMap,
                    std::unordered_map<Task*, bool>& visited);
};

}  // namespace mrh
