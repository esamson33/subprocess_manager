#include "subprocess_manager.hpp"
#include "threaded_manager.hpp"
#include "multiplexed_manager.hpp"
#include "logging.hpp"
#include <yaml-cpp/yaml.h>
#include <boost/algorithm/string.hpp>
#include <memory>
#include <iostream>
#include <thread>

std::shared_ptr<ISubprocessManager> make_manager(const std::string& type) {
    if (type == "threaded") {
        return std::make_shared<ThreadedSubprocessManager>();
    } else if (type == "multiplexed") {
        return std::make_shared<MultiplexedSubprocessManager>();
    } else {
        throw std::invalid_argument("Unknown manager type: " + type);
    }
}

int main() {
    setup_logger();
    spdlog::info("Starting subprocess manager...");

    YAML::Node config = YAML::LoadFile("config.yaml");
    std::string command = config["command"].as<std::string>();
    std::string manager_type = config["manager_type"] ? config["manager_type"].as<std::string>() : "threaded";

    auto manager = make_manager(manager_type);
    for (int i=0; i<1024; ++i) {
        manager->start_process(command, [i](int exit_code, const std::string& out, const std::string& err) {
            spdlog::info("{} Process exited with code {}", i, exit_code);
            auto out_c = boost::algorithm::trim_right_copy(out);
            spdlog::info("{} STDOUT: {}", i, out_c);
            spdlog::info("{} STDERR: {}", i, err);
        });
    }
    
    //std::this_thread::sleep_for(std::chrono::seconds(5));
    manager->shutdown();
}
