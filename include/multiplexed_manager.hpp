#pragma once

#include <spdlog/spdlog.h>
#include "subprocess_manager.hpp"
#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <spdlog/spdlog.h>
#include <thread>
#include <map>
#include <memory>

class MultiplexedSubprocessManager : public ISubprocessManager {
public:
    MultiplexedSubprocessManager()
        : io(), work_guard(io.get_executor()) {
        io_thread = std::thread([this]() { io.run(); });
    }

    ~MultiplexedSubprocessManager() override {
        shutdown();
    }

    void start_process(const std::string& command, ExitCallback on_exit) override {
        //spdlog::info("using multiplexed subprocess manager for command: {}", command);
        auto proc = std::make_shared<SubprocessContext>(io, command, std::move(on_exit));
        proc->start();
        processes.emplace(proc.get(), proc);
    }

    void shutdown() override {
        processes.clear();
        work_guard.reset();
        if (io_thread.joinable()) io_thread.join();
    }

private:
    struct SubprocessContext : public std::enable_shared_from_this<SubprocessContext> {
        SubprocessContext(boost::asio::io_context& io_ctx, const std::string& cmd, ExitCallback cb)
            : stdout_pipe(io_ctx),
              stderr_pipe(io_ctx),
              command(cmd),
              on_exit(std::move(cb)),
              io(io_ctx) {}

        void start() {
            child = boost::process::child(command,
                        boost::process::std_out > stdout_pipe,
                        boost::process::std_err > stderr_pipe,
                        boost::process::on_exit(
                            [self = shared_from_this()](int exit_code, const std::error_code&) {
                                self->on_exit(exit_code, self->stdout_str, self->stderr_str);
                            }),
                        io);
            read_stdout();
            read_stderr();
        }

        void read_stdout() {
            auto buf = std::make_shared<boost::asio::streambuf>();
            boost::asio::async_read_until(stdout_pipe, *buf, '\n',
                [this, buf](boost::system::error_code ec, std::size_t) {
                    if (!ec) {
                        std::istream is(buf.get());
                        std::string line;
                        std::getline(is, line);
                        stdout_str += line + "\n";
                        read_stdout();
                    }
                });
        }

        void read_stderr() {
            auto buf = std::make_shared<boost::asio::streambuf>();
            boost::asio::async_read_until(stderr_pipe, *buf, '\n',
                [this, buf](boost::system::error_code ec, std::size_t) {
                    if (!ec) {
                        std::istream is(buf.get());
                        std::string line;
                        std::getline(is, line);
                        stderr_str += line + "\n";
                        read_stderr();
                    }
                });
        }

        std::string command;
        ExitCallback on_exit;
        boost::asio::io_context& io;
        boost::process::child child;
        boost::process::async_pipe stdout_pipe, stderr_pipe;
        std::string stdout_str, stderr_str;
    };

    std::map<void*, std::shared_ptr<SubprocessContext>> processes;
    boost::asio::io_context io;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard;
    std::thread io_thread;
};
