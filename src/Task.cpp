#include "MRHelper/Task.hpp"

#include <mutex>

#include "MRHelper/ThreadPool.hpp"

namespace mrh {

Task::Task(bool cacheResult) : _cacheResult(cacheResult) {}

std::any Task::execute(ThreadPool& threadPool) {
    if (_cacheResult) {
        std::call_once(_onceFlag, [&]() {
            std::any res = runImpl(threadPool);
            {
                std::lock_guard<std::mutex> lock(_mutex);
                _result = res;
            }
            _condVar.notify_all();
        });
        std::unique_lock<std::mutex> lock(_mutex);
        return _result;
    } else {
        return runImpl(threadPool);
    }
}

void Task::setCacheResult(bool cacheResult) {
    _cacheResult = cacheResult;
}

bool Task::getCacheResult() const {
    return _cacheResult;
}

void Task::dependsOn(const std::shared_ptr<Task>& dependency) {
    if (dependency) {
        _dependencies.push_back(dependency);
        dependency->addDependent(shared_from_this());
    }
}

void Task::addDependent(const std::shared_ptr<Task>& dependent) {
    _dependents.push_back(dependent);
}

const std::vector<std::shared_ptr<Task>>& Task::getDependencies() const {
    return _dependencies;
}

const std::vector<std::weak_ptr<Task>>& Task::getDependents() const {
    return _dependents;
}

std::any Task::getResult() const {
    std::unique_lock<std::mutex> lock(_mutex);
    return _result;
}

}  // namespace mrh
