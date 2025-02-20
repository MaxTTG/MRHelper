#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace mrh {

/**
 * @brief A thread pool based on the producer-consumer pattern.
 */
class ThreadPool {
public:
    /**
     * @brief Constructs a ThreadPool.
     * @param numThreads Number of worker threads.
     */
    explicit ThreadPool(size_t numThreads);

    ~ThreadPool();

    /**
     * @brief Enqueues a task for execution.
     *
     * @tparam F Function type.
     * @tparam Args Argument types.
     * @param f The function to execute.
     * @param args Arguments for the function.
     * @return A future to the function result.
     */
    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>>;

    /**
     * @brief Attempts to execute one task from the queue in the current thread.
     *
     * @return True if a task was executed, false otherwise.
     */
    bool tryExecuteOne();

private:
    std::vector<std::thread> _workers;         ///< Worker threads.
    std::queue<std::function<void()>> _tasks;  ///< Task queue.
    std::mutex _queueMutex;                    ///< Protects _tasks.
    std::condition_variable _condition;        ///< Notifies worker threads.
    bool _stop;                                ///< Indicates if the pool is stopping.
};

template <class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>> {
    using return_type = typename std::invoke_result_t<F, Args...>;
    auto task =
        std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(_queueMutex);
        if (_stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");
        _tasks.emplace([task]() { (*task)(); });
    }
    _condition.notify_one();
    return res;
}

}  // namespace mrh
