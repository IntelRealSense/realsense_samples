// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#pragma once

#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <thread>
#include <functional>
#include <cassert>
#include <cstddef>
#include <memory>

namespace ConcurrencyUtils
{

// Simplest implementation of a blocking concurrent queue for thread messaging
template<class T>
class single_consumer_queue
{
    std::queue<T> q;
    std::mutex mutex;
    std::condition_variable cv;

public:
    single_consumer_queue<T>() : q(), mutex(), cv() {}

    void enqueue(T item)
    {
        std::unique_lock<std::mutex> lock(mutex);
        q.push(std::move(item));
        lock.unlock();
        cv.notify_one();
    }

    T dequeue()
    {
        std::unique_lock<std::mutex> lock(mutex);
        const auto ready = [this]()
        {
            return !q.empty();
        };
        cv.wait(lock, ready);
        auto item = std::move(q.front());
        q.pop();
        return std::move(item);
    }

    bool try_dequeue(T* item)
    {
        std::unique_lock<std::mutex> lock(mutex);
        if(q.size()>0)
        {
            auto val = std::move(q.front());
            q.pop();
            *item = std::move(val);
            return true;
        }
        return false;
    }

    void clear()
    {
        std::unique_lock<std::mutex> lock(mutex);
        while (q.size() > 0)
        {
            auto item = std::move(q.front());
            q.pop();
        }
    }
    size_t size()
    {
        std::unique_lock<std::mutex> lock(mutex);
        return q.size();
    }
};

inline bool any_costumers_alive(const std::vector<bool>& running)
{
    for (auto is_running : running)
    {
        if (is_running)
        {
            return true;
        }
    }
    return false;
}

template <typename T>
class MultiThreadRunQueue
{
    single_consumer_queue<T> q;
    std::atomic_bool running;

    std::vector<std::thread> threads;

    void run()
    {
        while (running)
        {
            T work = q.dequeue();
            if (running) work();
        }
    }

public:
    MultiThreadRunQueue() : running(false) {}

    ~ MultiThreadRunQueue()
    {
        if (running) stop();
    }

    void add(T&& work)
    {
        q.enqueue(std::move(work));
    }

    void start(int nthreads=1)
    {
        running = true;
        for (int i = 0; i < nthreads; i++)
        {
            threads.emplace_back([this]()
            {
                run();
            });
        }
    }

    void stop()
    {
        running = false;
        for (unsigned int i = 0; i < threads.size(); i++)
        {
            q.enqueue(T{}); // make deque return if empty
        }
        for (auto&& thread : threads)
        {
            thread.join();
        }
        q.clear();
    }
};

typedef MultiThreadRunQueue<std::function<void()>> WorkQueue;

}
