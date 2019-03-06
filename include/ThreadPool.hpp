//
// Created by Dániel Molnár on 2019-03-03.
//

#pragma once
#ifndef CORE_THREADPOOL_HPP
#define CORE_THREADPOOL_HPP

#include <iostream>
#include <functional>
#include <future>
#include <queue>
#include <shared_mutex>
#include <condition_variable>
#include <array>
#include <optional>
#include <cassert>

namespace Core {

template<unsigned N>
class ThreadPool {
private:
    template<class Callable, class ...Args>
    using ResultTypeOfCallable = typename std::invoke_result<Callable, Args...>::type;

    using WrappedJob = std::function<void(unsigned)>;
    class Worker {
    private:
        std::thread _thread;
        bool _occupied = false;
    public:
        Worker() = default;
        ~Worker() {
            if(_thread.joinable())
                _thread.detach();
        }

        explicit Worker(std::function<void()> job) noexcept : _thread(job), _occupied(true) {}

        Worker(const Worker&) = delete;
        Worker& operator=(const Worker&) = delete;

        Worker(Worker&& other) noexcept {
            if(_thread.joinable())
                _thread.join();

            _thread = std::move(other._thread);
            _occupied = other._occupied;
            other._occupied = false;
        }

        Worker& operator=(Worker&& other) {
            if(_thread.joinable())
                _thread.join();

            _thread = std::move(other._thread);
            _occupied = other._occupied;
            other._occupied = false;
            return *this;
        }

        void lock() {
            _occupied = true;
        }

        void free() {
            _occupied = false;
        }

        bool isFree() const {
            return !_occupied;
        }
    };

    mutable std::mutex _workerGuard;
    std::array<Worker, N> _workers;

    mutable std::shared_mutex _queueGuard;
    std::atomic_bool _stopped = false;
    std::atomic_bool _waitForFinish = true;
    std::queue<std::function<void(unsigned)>> _jobQueue;

    std::thread _poolingThread;
    std::condition_variable_any hasJobOrStopped;
    std::condition_variable hasUnoccupiedThread;

    WrappedJob takeNextJob() {
        std::unique_lock lock(_queueGuard);
        WrappedJob job = std::move(_jobQueue.front());
        _jobQueue.pop();

        return job;
    }

    std::optional<unsigned> getIdleWorker() const {
        std::unique_lock lock(_workerGuard);
        auto workerIt = std::find_if(_workers.begin(), _workers.end(), [](const auto &worker) {
            return worker.isFree();
        });

        return workerIt != _workers.end() ?
               std::optional<unsigned>(std::distance(_workers.begin(), workerIt)) :
               std::nullopt;
    }

    void mainLoop() {
        while (!_stopped || (_waitForFinish && !hasQueuedJob())) {
            if (hasQueuedJob()) {
                if (auto unoccupiedIdx = getIdleWorker(); unoccupiedIdx.has_value()) {
                    auto job = takeNextJob();

                    std::unique_lock lock(_workerGuard);
                    _workers.at(unoccupiedIdx.value()) = Worker(std::bind(job, unoccupiedIdx.value()));
                } else {
                    std::unique_lock lock(_workerGuard);
                    hasUnoccupiedThread.wait(lock);
                }
            } else {
                std::unique_lock lock(_queueGuard);
                hasJobOrStopped.wait(lock);
            }
        }

        for (auto &worker : _workers) {
            worker.free();
        }
    }
public:
    ThreadPool() {
        _poolingThread = std::thread(std::bind(&ThreadPool::mainLoop, this));
    }

    ~ThreadPool() {
        stop(false);
        _poolingThread.join();
    }

    void stop(bool waitForQueuedJobs) {
        _stopped = true;
        _waitForFinish = waitForQueuedJobs;
        hasJobOrStopped.notify_one();
    }

    bool hasQueuedJob() const {
        std::shared_lock lock(_queueGuard);
        return !_jobQueue.empty();
    }

    template<class Callable, class... Args>
    auto addJob(Callable job, Args ...args) -> std::future<ResultTypeOfCallable<Callable, Args...>> {
        auto task = std::make_shared<std::packaged_task<ResultTypeOfCallable<Callable, Args...>(Args...)>>(job);

        auto wrappedJob = [this, task, args = std::make_tuple(std::forward<Args>(args)...)](unsigned idx) {
            std::apply(*task, args);

            {
                std::unique_lock lock(_workerGuard);
                _workers.at(idx).free();
            }
            hasUnoccupiedThread.notify_one();
        };

        {
            std::unique_lock lock(_queueGuard);
            _jobQueue.push(wrappedJob);
        }
        hasJobOrStopped.notify_one();

        return task->get_future();
    }
};

}

#endif //CORE_THREADPOOL_HPP
