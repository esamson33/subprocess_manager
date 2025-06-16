#pragma once
#include <functional>
#include <string>

using ExitCallback = std::function<void(int, const std::string&, const std::string&)>;

class ISubprocessManager {
public:
    virtual ~ISubprocessManager() = default;
    virtual void start_process(const std::string& command, ExitCallback on_exit) = 0;
    virtual void shutdown() = 0;
};
