#pragma once

#include <any>
#include <chrono>
#include <functional>
#include <future>
#include <map>
#include <thread>
#include <utility>
#include <vector>

#include "MRHelper/Task.hpp"
#include "MRHelper/ThreadPool.hpp"

namespace mrh {

/**
 * @brief Template class for generic MapReduce tasks.
 *
 * @tparam Input  Type of input data (e.g. std::vector<T>).
 * @tparam Key    Type of keys produced by the map function.
 * @tparam Value  Type of values associated with keys.
 * @tparam Output Type of the output produced by the reduce function.
 */
template <typename Input, typename Key, typename Value, typename Output>
class MapReduceTask: public Task {
public:
    /// Type alias for the map function.
    using MapFunction = std::function<std::vector<std::pair<Key, Value>>(const Input&)>;
    /// Type alias for the reduce function.
    using ReduceFunction = std::function<Output(const Key&, const std::vector<Value>&)>;

    /**
     * @brief Constructs a MapReduceTask.
     *
     * @param mapFunc Map function.
     * @param reduceFunc Reduce function.
     * @param numMapTasks Number of parallel map tasks.
     * @param cacheResult If true, caches the result.
     */
    MapReduceTask(MapFunction mapFunc, ReduceFunction reduceFunc, int numMapTasks = 1, bool cacheResult = true)
        : Task(cacheResult)
        , _mapFunc(std::move(mapFunc))
        , _reduceFunc(std::move(reduceFunc))
        , _numMapTasks(numMapTasks) {}

protected:
    /**
     * @brief Executes the MapReduce task.
     *
     * @param threadPool Thread pool used for parallel execution.
     * @return A std::any containing a std::map<Key, Output> with the results.
     */
    virtual std::any runImpl(ThreadPool& threadPool) override {
        // Get input data from the first dependency if available.
        Input input{};
        if (!getDependencies().empty()) {
            input = std::any_cast<Input>(getDependencies().front()->execute(threadPool));
        } else {
            input = Input();
        }

        // Map phase: launch _numMapTasks parallel tasks.
        std::vector<std::future<std::vector<std::pair<Key, Value>>>> mapFutures;
        for (int i = 0; i < _numMapTasks; ++i) {
            mapFutures.push_back(threadPool.enqueue([this, input]() { return _mapFunc(input); }));
        }

        // Collect intermediate results.
        std::vector<std::pair<Key, Value>> intermediate;
        for (auto& fut : mapFutures) {
            while (fut.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
                threadPool.tryExecuteOne();
                std::this_thread::yield();
            }
            auto partial = fut.get();
            intermediate.insert(intermediate.end(), partial.begin(), partial.end());
        }

        // Shuffle phase: group by key.
        std::map<Key, std::vector<Value>> groups;
        for (auto& kv : intermediate) {
            groups[kv.first].push_back(kv.second);
        }

        // Reduce phase: launch parallel reduce tasks.
        std::vector<std::future<std::pair<Key, Output>>> reduceFutures;
        for (auto& group : groups) {
            reduceFutures.push_back(threadPool.enqueue([this, key = group.first, values = group.second]() {
                Output reducedValue = _reduceFunc(key, values);
                return std::make_pair(key, reducedValue);
            }));
        }

        // Collect reduce results.
        std::map<Key, Output> result;
        for (auto& fut : reduceFutures) {
            while (fut.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
                threadPool.tryExecuteOne();
                std::this_thread::yield();
            }
            auto kv          = fut.get();
            result[kv.first] = kv.second;
        }

        return result;
    }

private:
    MapFunction _mapFunc;        ///< Map function.
    ReduceFunction _reduceFunc;  ///< Reduce function.
    int _numMapTasks;            ///< Number of parallel map tasks.
};

}  // namespace mrh
