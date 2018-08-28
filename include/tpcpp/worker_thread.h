/*
 * worker_thread.h
 *
 *  Created on: 28 Aug 2018
 *      Author: Fabian Meyer
 */

#ifndef TPCPP_WORKER_THREAD_HPP_
#define TPCPP_WORKER_THREAD_HPP_

#include <functional>
#include <thread>
#include <memory>
#include <atomic>
#include "tpcpp/blocking_queue.h"

namespace tp
{
    typedef std::function<void(void)> Task;
    typedef BlockingQueue<Task> TaskQueue;

    class WorkerThread
    {
    private:
        std::atomic_bool keepRunning_;
        TaskQueue &queue_;
        std::thread thread_;

        void run()
        {
            while(keepRunning_)
            {
                Task task = queue_.pop();

                try
                {
                    if(task)
                        task();
                }
                catch(...)
                {
                    // ignore exceptions, is this wise?
                }
            }
        }

    public:
        WorkerThread(TaskQueue &queue)
            : keepRunning_(true), queue_(queue), thread_(&WorkerThread::run, this)
        {
        }

        ~WorkerThread()
        {
            stop();
        }

        void stop()
        {
            keepRunning_ = false;
        }

        void join()
        {
            thread_.join();
        }
    };
}

#endif
