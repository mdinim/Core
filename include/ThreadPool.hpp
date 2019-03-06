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

/// \brief Thread pool implementation.
/// \param N the maximum number of concurrent threads this instance is responsible for.
/// Any callable type can be dispatched into this construct.
/// Runs its own thread with the main loop waiting for jobs to process.
/// Result values can be claimed through std::future objects.
template<unsigned N>
class ThreadPool {
private:
    template<class Callable, class ...Args>
    using ResultTypeOfCallable = typename std::invoke_result<Callable, Args...>::type;

    using WrappedJob = std::function<void(unsigned)>;

    /// \brief Internal representation of a worker thread.
    class Worker {
    private:
        /// \brief The wrapped thread.
        std::thread _thread;

        /// \brief Flag that signals if the worker is occupied.
        bool _occupied = false;
    public:
        /// \brief Construct an empty worker (with nothing to do)
        Worker() = default;

        /// \brief Destruct the instance of the worker
        /// Detaches the underlying thread.
        ~Worker() {
            if(_thread.joinable())
                _thread.detach();
        }

        /// \brief Construct a worker with a job to do.
        /// The thread is occupied afterwards.
        explicit Worker(std::function<void()> job) noexcept : _thread(job), _occupied(true) {}

        /// \brief Disabled copy constructor.
        Worker(const Worker&) = delete;

        /// \brief Disabled copy-assignment operator.
        Worker& operator=(const Worker&) = delete;

        /// \brief Move constructor.
        /// Takes over the ownership of the \param other Worker.
        /// Finishes its current job first.
        Worker(Worker&& other) noexcept {
            if(_thread.joinable())
                _thread.join();

            _thread = std::move(other._thread);
            _occupied = other._occupied;
            other._occupied = false;
        }

        /// \brief Move assignment operator.
        /// Same as move constructor.
        Worker& operator=(Worker&& other) {
            if(_thread.joinable())
                _thread.join();

            _thread = std::move(other._thread);
            _occupied = other._occupied;
            other._occupied = false;
            return *this;
        }

        /// \brief Lock the worker.
        void lock() {
            _occupied = true;
        }

        /// \brief Free the worker.
        void free() {
            _occupied = false;
        }

        /// \brief Check if the worker is occupied.
        bool isFree() const {
            return !_occupied;
        }
    };

    /// \brief Guard for the worker array.
    mutable std::mutex _workerGuard;

    /// \brief Container for the Workers.
    std::array<Worker, N> _workers;

    /// \brief Guard for the job queue.
    mutable std::shared_mutex _queueGuard;

    /// \brief Atomic flag for ThreadPool stopping.
    std::atomic_bool _stopped = false;

    /// \brief Atomic flag if a stopped ThreadPool should finish its already queued jobs.
    std::atomic_bool _waitForFinish = true;

    /// \brief Container for the job queues.
    std::queue<std::function<void(unsigned)>> _jobQueue;

    /// \brief The ThreadPool's own thread it dispatches the jobs on.
    std::thread _poolingThread;

    /// \brief Condition variable to wait for stopping or jobs.
    std::condition_variable_any hasJobOrStopped;

    /// \brief Condition variable to wait finishing workers.
    std::condition_variable hasUnoccupiedWorker;

    /// \brief Retreive the next job to do.
    WrappedJob takeNextJob() {
        std::unique_lock lock(_queueGuard);
        WrappedJob job = std::move(_jobQueue.front());
        _jobQueue.pop();

        return job;
    }

    /// \brief Retreive the index of an idling Worker.
    std::optional<unsigned> getIdleWorker() const {
        std::unique_lock lock(_workerGuard);
        auto workerIt = std::find_if(_workers.begin(), _workers.end(), [](const auto &worker) {
            return worker.isFree();
        });

        return workerIt != _workers.end() ?
               std::optional<unsigned>(std::distance(_workers.begin(), workerIt)) :
               std::nullopt;
    }

    /// \brief Main loop that does the orchestration.
    void mainLoop() {
        while (!_stopped || (_waitForFinish && !hasQueuedJob())) {
            if (hasQueuedJob()) {
                if (auto unoccupiedIdx = getIdleWorker(); unoccupiedIdx.has_value()) {
                    auto job = takeNextJob();

                    std::unique_lock lock(_workerGuard);
                    _workers.at(unoccupiedIdx.value()) = Worker(std::bind(job, unoccupiedIdx.value()));
                } else {
                    std::unique_lock lock(_workerGuard);
                    hasUnoccupiedWorker.wait(lock);
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
    /// \brief Construct the thread pool.
    /// Launches its pooling thread.
    ThreadPool() {
        _poolingThread = std::thread(std::bind(&ThreadPool::mainLoop, this));
    }

    /// \brief Destruct the thread pool instance.
    /// Does not wait for its Workers to finish, does not clears its job queue first.
    ~ThreadPool() {
        stop(false);
        _poolingThread.join();
    }

    /// \brief Stop the thread pool.
    /// \param waitForQueuedJobs decides whether or not clear ist job queue before finishing.
    void stop(bool waitForQueuedJobs) {
        _stopped = true;
        _waitForFinish = waitForQueuedJobs;
        hasJobOrStopped.notify_one();
    }

    /// \brief Checks if it has any job waiting for a Worker.
    bool hasQueuedJob() const {
        std::shared_lock lock(_queueGuard);
        return !_jobQueue.empty();
    }

    /// \brief Add a job.
    /// \param job any callable (functor, lambda, std::function, function pointer).
    /// \param args parameters the callable should be invoked with.
    /// \returns std::future that'll get the result if its done.
    template<class Callable, class... Args>
    auto addJob(Callable job, Args ...args) -> std::future<ResultTypeOfCallable<Callable, Args...>> {
        auto task = std::make_shared<std::packaged_task<ResultTypeOfCallable<Callable, Args...>(Args...)>>(job);

        auto wrappedJob = [this, task, args = std::make_tuple(std::forward<Args>(args)...)](unsigned idx) {
            std::apply(*task, args);

            {
                std::unique_lock lock(_workerGuard);
                _workers.at(idx).free();
            }
            hasUnoccupiedWorker.notify_one();
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
