/*
 * blocking_queue.h
 *
 *  Created on: 24 Aug 2018
 *      Author: Fabian Meyer
 */

#ifndef TPOOL_BLOCKING_QUEUE_HPP_
#define TPOOL_BLOCKING_QUEUE_HPP_

#include <deque>
#include <mutex>
#include <condition_variable>

namespace tpool
{

    template<class T>
    class BlockingQueue
    {
    private:
        size_t maxSize_;
        volatile size_t waiting_;
        std::deque<T> queue_;
        std::mutex mutex_;
        std::condition_variable popCond_;
        std::condition_variable pushCond_;
        std::condition_variable waitCond_;

        bool _full() const
        {
            return maxSize_ > 0 && queue_.size() >= static_cast<unsigned int>(maxSize_);
        }

        bool _empty() const
        {
            return queue_.empty();
        }

    public:
        BlockingQueue()
            : BlockingQueue(0)
        {
        }

        BlockingQueue(const size_t maxSize)
            : maxSize_(maxSize), waiting_(0), queue_(), mutex_(), popCond_(),
            pushCond_(), waitCond_()
        {
        }

        ~BlockingQueue()
        {
        }

        bool full()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            return _full();
        }

        bool empty()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            return _empty();
        }

        void push(T obj)
        {
            std::unique_lock<std::mutex> lock(mutex_);

            pushCond_.wait(lock, [this](){return !this->_full();});

            queue_.push_back(obj);

            popCond_.notify_one();
        }

        T pop()
        {
            std::unique_lock<std::mutex> lock(mutex_);

            ++waiting_;
            waitCond_.notify_one();

            popCond_.wait(lock, [this](){return !this->_empty();});

            --waiting_;

            T result = queue_.front();
            queue_.pop_front();


            pushCond_.notify_one();

            return result;
        }

        void wait(const size_t cnt)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            waitCond_.wait(lock, [this, cnt](){return this->_empty() &&
                this->waiting_ >= cnt;});
        }

        void clear()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            queue_.clear();
        }
    };
}

#endif
