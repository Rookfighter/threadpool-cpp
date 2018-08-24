/*
 * blocking_queue.h
 *
 *  Created on: 24 Aug 2018
 *      Author: Fabian Meyer
 */

#ifndef POOL_BLOCKINGQUEUE_HPP_
#define POOL_BLOCKINGQUEUE_HPP_

#include <queue>
#include <mutex>
#include <condition_variable>

namespace pool
{

    template<class T>
    class BlockingQueue
    {
    private:
        size_t maxSize_;
        std::queue<T> queue_;
        std::mutex mutex_;
        std::condition_variable popCond_;
        std::condition_variable pushCond_;

        bool isFull()
        {
            return maxSize_ > 0 && queue_.size() >= static_cast<unsigned int>(maxSize_);
        }
    public:
        BlockingQueue()
            : BlockingQueue(0)
        {
        }

        BlockingQueue(const size_t maxSize)
            : maxSize_(maxSize), queue_(), mutex_(), popCond_(), pushCond_()
        {
        }

        ~BlockingQueue()
        {
        }

        bool full()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            return isFull();
        }

        bool empty()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            return queue_.empty();
        }

        void push(T obj)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while(isFull())
                pushCond_.wait(lock);
            queue_.push(obj);
            popCond_.notify_one();
        }

        T pop()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while(queue_.empty())
                popCond_.wait(lock);
            T result = queue_.front();
            queue_.pop();
            pushCond_.notify_one();
            return result;
        }
    };
}

#endif
