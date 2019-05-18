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
/// Result values can be claimed through std::future objects.
template<unsigned N>
class ThreadPool {
private:
    template<class Callable, class ...Args>
    using ResultTypeOfCallable = typename std::invoke_result<Callable, Args...>::type;

    using WrappedJob = std::function<void()>;
    using JobQueue = std::queue<WrappedJob>;

    /// \brief Internal representation of a worker thread.
    /// Subscribed to the job queue, whenever a job arrives one of the workers pick it up.
    /// If the Threadpool is destroyed, or stopped, the queue is not emptied, there may be leftover jobs.
    class Worker {
    private:
        /// \brief The wrapped thread.
        std::thread _thread;

        /// \brief Atomic flag for Worker stopping.
        std::shared_ptr<std::atomic_bool> _stopped = std::make_shared<std::atomic_bool>(false);

        /// \brief After stopping the worker if this is true, the worker does not stop until the queue is cleared.
        std::shared_ptr<std::atomic_bool> _clearQueue = std::make_shared<std::atomic_bool>(false);

        /// \brief Static loop function that actually is executed on the worker threads
        static void mainLoop (std::shared_ptr<std::atomic_bool> stopped, std::shared_ptr<std::atomic_bool> clearQueue,
                    std::shared_ptr<JobQueue> queue,
                    std::shared_ptr<std::shared_mutex> queueGuard,
                    std::shared_ptr<std::condition_variable_any> hasJobOrStopped) {
            auto hasJob = [queue, queueGuard] () {
                std::shared_lock queueLock(*queueGuard);
                return !queue->empty();
            };
            while(!*stopped || (*clearQueue && hasJob())) {
                std::optional<WrappedJob> job;
                {
                    std::unique_lock queueLock(*queueGuard);
                    hasJobOrStopped->wait(queueLock, [&stopped, &queue]() {
                        return *stopped || !queue->empty();
                    });

                    if (*stopped && (!*clearQueue || queue->empty())) {
                        break;
                    }

                    if (!queue->empty()) {
                        job = std::move(queue->front());
                        queue->pop();
                    }
                }

                if(job.has_value())
                    job.value()();
            }
        }
    public:
        /// \brief Construct an empty worker (with nothing to do)
        Worker(std::shared_ptr<JobQueue> queue, std::shared_ptr<std::shared_mutex> queueGuard,
                std::shared_ptr<std::condition_variable_any> hasJobOrStopped)
        {
            _thread = std::thread(std::bind(&Worker::mainLoop, _stopped, _clearQueue,
                    queue, queueGuard, hasJobOrStopped));
        }

        /// \brief Destruct the instance of the worker
        /// Detaches the underlying thread.
        ~Worker() {
            if(!_stopped)
                stop();

            if(_thread.joinable())
                _thread.detach();
        }

        /// \brief Disabled copy constructor.
        Worker(const Worker&) = delete;

        /// \brief Disabled copy-assignment operator.
        Worker& operator=(const Worker&) = delete;

        /// \brief Disabled move constructor.
        Worker(Worker&& other) = delete;

        /// \brief Disabled move assignment operator.
        Worker& operator=(Worker&& other) = delete;

        /// \brief Stop the worker.
        /// \param graceful keep the worker active until the job queue is cleared.
        void stop(bool graceful = false) {
            *_stopped = true;
            *_clearQueue = graceful;
        }
    };

    /// \brief Container for the Workers.
    std::vector<std::unique_ptr<Worker>> _workers;

    /// \brief Guard for the job queue.
    std::shared_ptr<std::shared_mutex> _queueGuard = std::make_shared<std::shared_mutex>();

    /// \brief Container for the job queues.
    std::shared_ptr<JobQueue> _jobQueue = std::make_shared<JobQueue>();

    /// \brief Flag if the pool is running
    std::atomic_bool _stopped = false;

    /// \brief Condition variable to wait for stopping or jobs.
    std::shared_ptr<std::condition_variable_any> hasJobOrStopped = std::make_shared<std::condition_variable_any>();
public:
    /// \brief Construct the thread pool.
    /// Launches its pooling thread.
    ThreadPool() {
        _workers.reserve(N);
        for(unsigned i = 0; i < N; ++i) {
            _workers.emplace_back(std::make_unique<Worker>(_jobQueue, _queueGuard, hasJobOrStopped));
        }
    }

    /// \brief Destruct the thread pool instance.
    /// Does not wait for its Workers to finish, does not clears its job queue first.
    ~ThreadPool() {
        _workers.clear();
        hasJobOrStopped->notify_all();
    }

    /// \brief Stop the thread pool.
    /// \param waitForQueuedJobs decides whether or not clear ist job queue before finishing.
    void stop(bool graceful = false) {
        _stopped = true;
        for (const auto& worker : _workers) {
            worker->stop(graceful);
        }

        hasJobOrStopped->notify_all();
    }

    /// \brief Checks if it has any job waiting for a Worker.
    bool hasQueuedJob() const {
        std::shared_lock lock(*_queueGuard);
        return !_jobQueue->empty();
    }

    /// \brief Add a job.
    /// \param job any callable (functor, lambda, std::function, function pointer).
    /// \param args parameters the callable should be invoked with.
    /// \returns std::future that'll get the result if its done.
    template<class Callable, class... Args>
    auto addJob(Callable job, Args ...args) -> std::future<ResultTypeOfCallable<Callable, Args...>> {
        if (_stopped) {
            return {};
        }

        auto task = std::make_shared<std::packaged_task<ResultTypeOfCallable<Callable, Args...>(Args...)>>(job);

        auto wrappedJob = [task, args = std::make_tuple(std::forward<Args>(args)...)]() {
            std::apply(*task, args);
        };

        {
            std::unique_lock lock(*_queueGuard);
            _jobQueue->push(wrappedJob);
        }
        hasJobOrStopped->notify_one();

        return task->get_future();
    }
};

}

#endif //CORE_THREADPOOL_HPP