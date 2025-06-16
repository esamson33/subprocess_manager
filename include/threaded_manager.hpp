#pragma once
#include "subprocess_manager.hpp"
#include <spdlog/spdlog.h>
#include <thread>
#include <vector>
#include <boost/process.hpp>

class ThreadedSubprocessManager : public ISubprocessManager {
public:

    ~ThreadedSubprocessManager() override{
        shutdown();
    }

    void start_process(const std::string& command, ExitCallback on_exit) override {
        //spdlog::info("using threaded subprocess manager for command: {}", command);
        threads.emplace_back([=]() {
            namespace bp = boost::process;
            bp::ipstream out, err;
            bp::child child(command, bp::std_out > out, bp::std_err > err);

            std::string stdout_str, stderr_str, line;
            while (child.running()) {
                if (std::getline(out, line)) stdout_str += line + "\n";
                if (std::getline(err, line)) stderr_str += line + "\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            child.wait();
            on_exit(child.exit_code(), stdout_str, stderr_str);
        });
    }

    void shutdown() override {
        for (auto& t : threads) if (t.joinable()) t.join();
    }

private:
    std::vector<std::thread> threads;
};
