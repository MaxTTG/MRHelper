#include "MRHelper/SimpleTask.hpp"

namespace mrh {

std::any SimpleTask::runImpl(ThreadPool& threadPool) {
    std::vector<std::any> inputs;
    for (const auto& dep : getDependencies()) {
        inputs.push_back(dep->execute(threadPool));
    }
    return _func(inputs);
}

}  // namespace mrh
