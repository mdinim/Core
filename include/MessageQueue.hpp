//
// Created by Dániel Molnár on 2019-02-20.
//

#pragma once
#ifndef CORE_MESSAGEQUEUE_HPP
#define CORE_MESSAGEQUEUE_HPP

#include <mutex>
#include <condition_variable>
#include <queue>

namespace Core {

/// \brief \enum for Message Priority
/// Any number can be used for arbitrary priority, these are just reference numbers
/// So users of the construct have the ability to use them.
enum class MessagePriority : unsigned {
    Low = 0,
    Normal = 50,
    High = 100
};

/// \brief Message queue template class.
/// Thread safe implementation of a message queue.
/// Any type can be passed through it.
template<class MessageContent>
class MessageQueue {
private:
    /// \brief Message wrapper (internal) type.
    /// Wraps the MessageContent type with its dispatched priority.
    struct MessageType {
        /// \brief Holds the priority of the message
        unsigned priority;

        /// \brief Holds the content of the message
        MessageContent content;

        /// \brief Construct the wrapper type.
        /// \param priority holds the priority value of the message.
        /// \param args holds the constructor arguments for the wrapped type.
        template<class ...Args>
        MessageType(unsigned priority, Args &&... args) : priority(priority),
                                                          content(std::forward<Args>(args)...) {}

        /// \brief Comparison operator for the underlying priority queue.
        bool operator<(const MessageType &rhs) const {
            return priority < rhs.priority;
        }

        /// \brief Equality operator for the unterlying priority queue.
        bool operator==(const MessageType &rhs) const {
            return priority == rhs.priority;
        }
    };

    /// \brief Guards the priority queue.
    mutable std::mutex _queueGuard;

    /// \brief Condition variable to have the ability for users to wait for the next message.
    mutable std::condition_variable conditionVariable;

    /// \brief Container of the messages.
    std::priority_queue<MessageType> _queue;
public:
    /// \brief Default construct the empty priority queue.
    MessageQueue() = default;

    /// \brief Disabled copy constructor.
    MessageQueue(const MessageQueue &) = delete;

    /// \brief Disabled copy-assignment operator.
    MessageType& operator=(const MessageQueue &) = delete;

    /// \brief Check if the queue is empty.
    bool empty() const {
        return _queue.empty();
    }

    /// \brief Queue messages with default priority (\see MessagePriority::Normal)
    template<class ...Args>
    void push(Args &&... args) {
        push(MessagePriority::Normal, std::forward<Args>(args)...);
    }

    /// \brief Queue messages with explicit pre-defined \param priority.
    template<class ...Args>
    void push(MessagePriority priority, Args &&... args) {
        std::unique_lock guard(_queueGuard);
        _queue.emplace(static_cast<unsigned>(priority), std::forward<Args>(args)...);
        conditionVariable.notify_one();
    }

    /// \brief Queue messages with arbitrary \param priority.
    template<class ...Args>
    void push(unsigned priority, Args &&... args) {
        std::unique_lock guard(_queueGuard);
        _queue.emplace(priority, std::forward<Args>(args)...);
        conditionVariable.notify_one();
    }

    /// \brief Retreive the highest-priority message.
    /// \returns The content of the message.
    MessageContent take() {
        std::unique_lock guard(_queueGuard);

        auto message = _queue.top();
        _queue.pop();
        return message.content;
    }

    /// \brief Suspend the thread until the next message arrives.
    void waitForMessage() const {
        std::unique_lock guard(_queueGuard);
        conditionVariable.wait(guard, [this]() {
            return !empty();
        });
    }
};

}

#endif //CORE_MESSAGEQUEUE_HPP
