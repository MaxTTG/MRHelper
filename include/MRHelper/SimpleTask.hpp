#pragma once

#include <any>
#include <functional>
#include <utility>

#include "MRHelper/Task.hpp"

namespace mrh {

/**
 * @brief A simple task that executes a user-supplied function.
 */
class SimpleTask: public Task {
public:
    /// Type alias for the task function.
    using TaskFunction = std::function<std::any(const std::vector<std::any>&)>;

    /**
     * @brief Constructs a SimpleTask.
     *
     * @param func The function to execute.
     * @param cacheResult Whether to cache the result.
     */
    explicit SimpleTask(TaskFunction func, bool cacheResult = true) : Task(cacheResult), _func(std::move(func)) {}

protected:
    /**
     * @brief Executes the task function.
     *
     * @param threadPool Thread pool used for dependency execution.
     * @return The result of the function.
     */
    virtual std::any runImpl(ThreadPool& threadPool) override;

private:
    TaskFunction _func;  ///< User-supplied function.
};

}  // namespace mrh
