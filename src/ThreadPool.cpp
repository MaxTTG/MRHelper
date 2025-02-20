#include "MRHelper/ThreadPool.hpp"

#include <utility>

namespace mrh {

ThreadPool::ThreadPool(size_t numThreads) : _stop(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        _workers.emplace_back([this] {
            for (;;) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->_queueMutex);
                    this->_condition.wait(lock, [this] { return _stop || !_tasks.empty(); });
                    if (_stop && _tasks.empty())
                        return;
                    task = std::move(_tasks.front());
                    _tasks.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(_queueMutex);
        _stop = true;
    }
    _condition.notify_all();
    for (std::thread &worker : _workers)
        worker.join();
}

bool ThreadPool::tryExecuteOne() {
    std::function<void()> task;
    {
        std::unique_lock<std::mutex> lock(_queueMutex);
        if (_tasks.empty())
            return false;
        task = std::move(_tasks.front());
        _tasks.pop();
    }
    task();
    return true;
}

}  // namespace mrh
