/*
 * thread_pool.h
 *
 *  Created on: 24 Aug 2018
 *      Author: Fabian Meyer
 */

#ifndef TPCPP_THREADPOOL_H_
#define TPCPP_THREADPOOL_H_

#include "tpcpp/worker_thread.h"

namespace tp
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
            for(unsigned int i = 0; i < cnt; ++i)
                threads_[i] = new WorkerThread(queue_);
        }

        ~ThreadPool()
        {
            stop();
        }

        void stop()
        {
            // send stop signal to each thread
            for(WorkerThread *thread : threads_)
                thread->stop();

            // clear the task queue
            clear();

            // push some dummy work for each worker thread
            for(unsigned int i = 0; i < threads_.size(); ++i)
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
            while(!queue_.empty())
                queue_.waiting(threads_.size());
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

}

#endif
