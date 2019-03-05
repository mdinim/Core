//
// Created by Dániel Molnár on 2019-02-20.
//

#pragma once
#ifndef CORE_MESSAGEQUEUE_HPP
#define CORE_MESSAGEQUEUE_HPP

#include <mutex>
#include <queue>

namespace Core {

enum class MessagePriority : unsigned {
    Low = 0,
    Normal = 50,
    High = 100
};

template<class MessageContent>
class MessageQueue {
private:
    struct MessageType {
        unsigned priority;
        MessageContent content;

        template<class ...Args>
        MessageType(unsigned priority, Args &&... args) : priority(priority),
                                                          content(std::forward<Args>(args)...) {}

        bool operator<(const MessageType &rhs) const {
            return priority < rhs.priority;
        }

        bool operator==(const MessageType &rhs) const {
            return priority == rhs.priority;
        }
    };

    std::mutex _queueGuard;
    std::condition_variable conditionVariable;
    std::priority_queue<MessageType> _queue;
public:
    MessageQueue() = default;

    MessageQueue(const MessageQueue &) = delete;

    bool empty() const {
        return _queue.empty();
    }

    template<class ...Args>
    void push(Args &&... args) {
        push(MessagePriority::Normal, std::forward<Args>(args)...);
    }

    template<class ...Args>
    void push(MessagePriority priority, Args &&... args) {
        std::unique_lock guard(_queueGuard);
        _queue.emplace(static_cast<unsigned>(priority), std::forward<Args>(args)...);
        conditionVariable.notify_one();
    }

    template<class ...Args>
    void push(unsigned priority, Args &&... args) {
        std::unique_lock guard(_queueGuard);
        _queue.emplace(priority, std::forward<Args>(args)...);
        conditionVariable.notify_one();
    }

    MessageContent take() {
        std::unique_lock guard(_queueGuard);

        auto message = _queue.top();
        _queue.pop();
        return message.content;
    }

    void pop() {
        std::unique_lock guard(_queueGuard);
    }

    void waitForMessage() {
        std::unique_lock guard(_queueGuard);
        conditionVariable.wait(guard, [this]() {
            return !empty();
        });
    }
};

}

#endif //CORE_MESSAGEQUEUE_HPP
