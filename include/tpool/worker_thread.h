/*
 * worker_thread.h
 *
 *  Created on: 28 Aug 2018
 *      Author: Fabian Meyer
 */

#ifndef TPOOL_WORKER_THREAD_HPP_
#define TPOOL_WORKER_THREAD_HPP_

#include <functional>
#include <thread>
#include <memory>
#include <atomic>
#include <iostream>
#include "tpool/blocking_queue.h"

namespace tpool
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
                catch(const std::exception &e)
                {
                    std::cerr << "WorkerThread: uncaught exception '" <<
                        e.what() << "'" << std::endl;
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
