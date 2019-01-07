/*
 * thread_pool.h
 *
 *  Created on: 24 Aug 2018
 *      Author: Fabian Meyer
 */

#ifndef TPOOL_THREADPOOL_H_
#define TPOOL_THREADPOOL_H_

#include "tpool/worker_thread.h"

namespace tpool
{
    class ThreadPool
    {
    private:
        TaskQueue queue_;
        std::vector<WorkerThread*> threads_;

    public:
        ThreadPool(const size_t cnt = 2, const size_t maxWork = 0)
            : queue_(maxWork), threads_(cnt)
        {
            for(size_t i = 0; i < cnt; ++i)
                threads_[i] = new WorkerThread(queue_);
        }

        ~ThreadPool()
        {
            stop();
            join();
            for(size_t i = 0; i < threads_.size(); ++i)
                delete threads_[i];
        }

        void stop()
        {
            // send stop signal to each thread
            for(WorkerThread *thread : threads_)
                thread->stop();

            // clear the task queue
            clear();

            // push some dummy work for each worker thread
            for(size_t i = 0; i < threads_.size(); ++i)
                queue_.push([](){});
        }

        void join()
        {
            // wait for threads to join
            for(WorkerThread *thread : threads_)
                thread->join();
        }

        void wait()
        {
            queue_.wait(threads_.size());
        }

        void enqueue(const Task &task)
        {
            queue_.push(task);
        }

        void clear()
        {
            queue_.clear();
        }

        size_t size() const
        {
            return threads_.size();
        }
    };

    template<typename Item, typename List=std::vector<Item>>
    void foreach(ThreadPool &pool,
        const std::function<void(Item&)> &func,
        List &list)
    {
        for(Item &item: list)
            pool.enqueue(std::bind(func, std::ref(item)));
        pool.wait();
    }

    template<typename Item, typename List=std::vector<Item>>
    void foreach(ThreadPool &pool,
        const std::function<void(const Item&)> &func,
        const List &list)
    {
        for(const Item &item: list)
            pool.enqueue(std::bind(func, std::cref(item)));
        pool.wait();
    }

    template<typename Index=size_t>
    void forindex(ThreadPool &pool, const std::function<void(const Index)> func, const Index cnt)
    {
        for(Index i = 0; i < cnt; ++i)
            pool.enqueue(std::bind(func, i));
        pool.wait();
    }

}

#endif
