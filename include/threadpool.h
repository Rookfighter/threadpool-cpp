
#ifndef POOL_THREADPOOL_H_
#define POOL_THREADPOOL_H_

#include <functional>
#include <memory>
#include "blocking_queue.h"

namespace pool
{
    typedef std::function<void()> Work;

    class WorkerThread
    {
    private:
        volatile bool keepRunning_;
        BlockingQueue<Work> &queue_;
        std::shared_ptr<std::thread> thread_;

        void run()
        {
            while(keepRunning_)
            {
                try
                {
                    while(keepRunning_)
                    {
                        Work work = queue_.pop();
                        if(work)
                            work();
                    }
                }
                catch(std::exception &e)
                {

                }
                catch(...)
                {

                }
            }
        }

    public:
        WorkerThread(BlockingQueue<Work> &queue)
            : keepRunning_(false), queue_(queue), thread_(nullptr)
        {
        }

        ~WorkerThread()
        {
        }

        void start()
        {
            if(keepRunning_)
                return;
            keepRunning_ = true;
            thread_ = std::shared_ptr<std::thread>(new std::thread(
                &WorkerThread::run, this));
        }

        void stop()
        {
            keepRunning_ = false;
        }

        void join()
        {
            thread->join();
        }
    };

    class ThreadPool
    {
    private:
        class WorkerThread;

        BlockingQueue<Work> queue_;
        std::vector<WorkerThread> threads_;
    public:
        ThreadPool(const size_t cnt = 2, const size_t maxWork = 0)
            : queue(maxWork), threads(cnt, WorkerThread(queue_)
        {
            start();
        }

        ~ThreadPool()
        {
            stop();
        }

        void start()
        {
        
            for(WorkerThread &thread : threads_)
                thread.start();
        }

        void stop()
        {
            // send stop signal to each thread
            for(WorkerThread &thread : threads_)
                thread.stop();

            clear();

            // push some dummy work for each worker thread
            for(unsigned int i = 0; i < threads_.size(); ++i)
                queue_.push([](){});

            join();
        }

        void join()
        {
            // wait for threads to join
            for(WorkerThread &thread : threads_)
                thread.join();
        }

        void enqueue(const Work &work)
        {
            queue_.push(work);
        }

        void clear()
        {
            queue_.clear()
        }
    };

}

#endif
