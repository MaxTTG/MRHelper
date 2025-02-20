#pragma once

#include <any>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

namespace mrh {

/**
 * @brief Base class for tasks with dependency management and optional result caching.
 */
class Task: public std::enable_shared_from_this<Task> {
public:
    /**
     * @brief Constructs a Task.
     * @param cacheResult If true, caches the result of execution.
     */
    Task(bool cacheResult = true);

    virtual ~Task() = default;

    /**
     * @brief Executes the task.
     *
     * If caching is enabled, subsequent calls return the cached result.
     *
     * @param threadPool Thread pool for executing dependencies.
     * @return The result as std::any.
     */
    std::any execute(class ThreadPool& threadPool);

    /**
     * @brief Sets the caching mode.
     * @param cacheResult True to cache the result.
     */
    void setCacheResult(bool cacheResult);

    /**
     * @brief Returns the current caching mode.
     * @return True if caching is enabled.
     */
    bool getCacheResult() const;

    /**
     * @brief Adds a dependency.
     * @param dependency The task this task depends on.
     */
    void dependsOn(const std::shared_ptr<Task>& dependency);

    /**
     * @brief Adds a dependent task.
     * @param dependent The task that depends on this task.
     */
    void addDependent(const std::shared_ptr<Task>& dependent);

    /**
     * @brief Returns the list of dependencies.
     * @return A vector of shared_ptr to tasks.
     */
    const std::vector<std::shared_ptr<Task>>& getDependencies() const;

    /**
     * @brief Returns the list of dependent tasks.
     * @return A vector of weak_ptr to tasks.
     */
    const std::vector<std::weak_ptr<Task>>& getDependents() const;

    /**
     * @brief Retrieves the result of the task.
     * @return The result as std::any.
     */
    std::any getResult() const;

protected:
    /**
     * @brief The task-specific execution logic.
     *
     * Must be implemented by derived classes.
     *
     * @param threadPool Thread pool for executing sub-tasks.
     * @return The result as std::any.
     */
    virtual std::any runImpl(class ThreadPool& threadPool) = 0;

private:
    std::vector<std::shared_ptr<Task>> _dependencies;  ///< Dependencies.
    std::vector<std::weak_ptr<Task>> _dependents;      ///< Dependent tasks.
    std::any _result;                                  ///< Cached result.
    std::once_flag _onceFlag;                          ///< Ensures single execution if caching.
    bool _cacheResult;                                 ///< Caching flag.
    mutable std::mutex _mutex;                         ///< Protects _result.
    std::condition_variable _condVar;                  ///< For synchronizing result access.
};

}  // namespace mrh
