/* threadpool.h
 *
 *  Created on: 21 Feb 2020
 *      Author: Fabian Meyer
 *     License: MIT
 */

#ifndef TRHEADPOOLCPP_H_
#define TRHEADPOOLCPP_H_

#include <deque>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <thread>
#include <memory>
#include <vector>
#include <chrono>

namespace tpcpp
{
    /** Queue implementation that blocks the dequeueing trhead when the queue is empty.
      * The queue can also maintain a maximum size and blocks a queueing thread if the
      * queue is full. */
    template<class T>
    class BlockingQueue
    {
    private:
        size_t maxSize_;
        std::deque<T> queue_;
        std::mutex mutex_;
        std::condition_variable popCond_;
        std::condition_variable pushCond_;

        bool _full() const
        {
            return maxSize_ > 0 && queue_.size() >= static_cast<unsigned int>(maxSize_);
        }

        bool _empty() const
        {
            return queue_.empty();
        }

    public:

        /** Creates a blocking queue with unlimited maximum size. */
        BlockingQueue()
            : BlockingQueue(0)
        {
        }

        /** Create a blocking queue with the given maximum size.
          * Set maxSize to 0 for unlimted size.
          * @param maxSize maximum size of the queue */
        BlockingQueue(const size_t maxSize)
            : maxSize_(maxSize), queue_(), mutex_(), popCond_(),
            pushCond_()
        {
        }

        /** Returns true if the queue is full.
          * Queues with unlimited maximum size are never full.
          * @return true if the queue is full, else false */
        bool full()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            return _full();
        }

        /** Returns true if the queue is empty.
          * @return true if the queue is empty, else false */
        bool empty()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            return _empty();
        }

        /** Returns the current number of elements within the queue .
          * @return number of elements in the queue */
        size_t size()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            return queue_.size();
        }

        /** Appends the given object to the end of the queue.
          * Blocks if the queue is full.
          * @param obj object to be appended to the queue */
        void enqueue(T obj)
        {
            std::unique_lock<std::mutex> lock(mutex_);

            pushCond_.wait(lock, [this](){return !this->_full();});

            queue_.push_back(obj);

            popCond_.notify_one();
        }

        /** Removes and returns the object in front of the queue.
          * Blocks if the queue is empty.
          * @return the removed object */
        T dequeue()
        {
            std::unique_lock<std::mutex> lock(mutex_);

            popCond_.wait(lock, [this](){return !this->_empty();});

            T result = queue_.front();
            queue_.pop_front();

            pushCond_.notify_one();

            return result;
        }

        /** Removes all elements from the queue. */
        void clear()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            queue_.clear();
        }

        /** Removes all elements from the queue and calls the given callback for each element. */
        template<typename Callback>
        void clear(const Callback &cb)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while(!queue_.empty())
            {
                T element = queue_.front();
                queue_.pop_front();
                cb(element);
            }
        }
    };


    /** A single work item that can be processed by a thread pool. */
    class Work
    {
    public:
        typedef std::shared_ptr<Work> Ptr;
        typedef std::shared_ptr<const Work> ConstPtr;

        enum class State
        {
            None,
            Waiting,
            Running,
            Completed,
            Errored,
            Cancelled
        };
    private:
        friend class WorkerThread;
        friend class ThreadPool;

        std::atomic<State> state_;
        std::function<void()> func_;
        std::condition_variable cond_;
        std::mutex mutex_;

        /** Sets the state of this work item and notifies waiting tasks.
          * @param state state that should be applied */
        void setState(State state)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            state_ = state;
            cond_.notify_all();
        }

        /** Executes this work item. */
        void execute()
        {
            setState(State::Running);

            if(func_)
                func_();

            setState(State::Completed);
        }

    public:
        Work(const std::function<void()> &func)
            : state_(State::None), func_(func), cond_(), mutex_()
        { }

        /** Returns true if the work item is in waiting state.
          * @return true if the work item is in waiting state, else false */
        bool waiting() const
        {
            return state_ == State::Waiting;
        }

        /** Returns true if the work item is in running state.
          * @return true if the work item is in running state, else false */
        bool running() const
        {
            return state_ == State::Running;
        }

        /** Returns true if the work item is in completed state.
          * @return true if the work item is in completed state, else false */
        bool completed() const
        {
            return state_ == State::Completed;
        }

        /** Returns true if the work item is in errored state.
          * @return true if the work item is in errored state, else false */
        bool errored() const
        {
            return state_ == State::Errored;
        }

        /** Returns true if the work item is in cancelled state.
          * @return true if the work item is in cancelled state, else false */
        bool cancelled() const
        {
            return state_ == State::Cancelled;
        }

        /** Returns true if the work item is in any post run state.
          * @return true if the work item is in any post run state, else false */
        bool stopped() const
        {
            return state_ == State::Completed ||
                   state_ == State::Errored ||
                   state_ == State::Cancelled;
        }

        /** Returns the curren state of the work item.
          * @return state of the work item */
        State state() const
        {
            return state_;
        }

        /** Blocks until the work item was executed and enters a post run state. */
        void wait()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_.wait(lock, [this]() { return this->stopped(); });
        }

        /** Blocks until the work item was executed and enters a post run state or the timeout expired. */
        template <class Rep, class Period>
        void wait(const std::chrono::duration<Rep, Period> &timeout)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_.wait_for(lock, [this]() { return this->stopped(); }, timeout);
        }
    };

    template<typename WorkList>
    void waitAll(const WorkList &works)
    {
        for(Work::Ptr work : works)
            work->wait();
    }


    /** Definition of a blocking qeue containing work items. */
    typedef BlockingQueue<Work::Ptr> WorkQueue;

    /** A worker thread that executes work items from a blocking queue. */
    class WorkerThread
    {
    public:
        typedef std::shared_ptr<WorkerThread> Ptr;
        typedef std::shared_ptr<const WorkerThread> ConstPtr;

        enum class State
        {
            Initializing,
            Running,
            Stopped,
        };

        /** Definition for callbacks, which are called when a exception occurs, while
          * work is executed. */
        typedef std::function<void(const WorkerThread&, const Work&, const std::exception &)> ExceptionCallback;

        /** Definition for callbacks, which are called when a undefined eror occurs, while
          * work is executed. */
        typedef std::function<void(const WorkerThread&, const Work&)> ErrorCallback;

    private:
        std::atomic<State> state_;
        size_t id_;
        std::atomic_bool keepRunning_;
        WorkQueue &queue_;
        std::thread thread_;

        ExceptionCallback exceptionCallback_;
        ErrorCallback errorCallback_;

        void run()
        {
            state_ = State::Running;

            while(keepRunning_)
            {
                // try to extract some work
                // blocks if nothing is available
                auto work = queue_.dequeue();

                // if the thread was stopped then
                // break here and do not execute the work
                if(!keepRunning_)
                {
                    work->setState(Work::State::Cancelled);
                    break;
                }

                try
                {
                    work->execute();
                }
                catch(const std::exception &e)
                {
                    work->setState(Work::State::Errored);
                    if(exceptionCallback_)
                        exceptionCallback_(*this, *work, e);
                }
                catch(...)
                {
                    work->setState(Work::State::Errored);
                    if(errorCallback_)
                        errorCallback_(*this, *work);
                }
            }

            state_ = State::Stopped;
        }

    public:
        WorkerThread(size_t id, WorkQueue &queue)
            : WorkerThread(id, queue,
                [](const WorkerThread&, const Work&, const std::exception &){},
                [](const WorkerThread&, const Work&){})
        { }

        WorkerThread(size_t id, WorkQueue &queue, const ExceptionCallback &exceptionCallback, const ErrorCallback &errorCallback)
            : state_(State::Initializing), id_(id), keepRunning_(true), queue_(queue), thread_(&WorkerThread::run, this),
              exceptionCallback_(exceptionCallback), errorCallback_(errorCallback)
        { }

        ~WorkerThread()
        {
            stop();
        }

        /** Returns the current state of the worker thread.
          * @return current state of the worker thread */
        State state() const
        {
            return state_;
        }

        /** Returns true if the worker thread is in running state.
          * @return true if the worker thread is running, else false */
        bool running() const
        {
            return state_ == State::Running;
        }

        /** Returns true if the worker thread is in stopped state.
          * @return true if the worker thread is stopped, else false */
        bool stopped() const
        {
            return state_ == State::Stopped;
        }

        /** Sends a stop signal to the worker thread.
          * Note this does not wait until the thread has stopped.
          * A successive call to join is required.
          * Also this does not create a dummy work. */
        void stop()
        {
            keepRunning_ = false;
        }

        /** Waits for this thread to end. */
        void join()
        {
            thread_.join();
        }
    };

    class ThreadPool
    {
    private:
        WorkQueue queue_;
        std::vector<WorkerThread::Ptr> threads_;

        void resize(const size_t threads)
        {
            size_t cnt = threads;
            if(cnt == 0)
            {
                cnt = std::thread::hardware_concurrency();
                if(cnt == 0)
                    cnt = 2;
                threads_.resize(cnt);
            }

            for(size_t i = 0; i < threads_.size(); ++i)
                threads_[i] = std::make_shared<WorkerThread>(i, queue_);
        }

    public:
        ThreadPool()
            : ThreadPool(0)
        { }

        ThreadPool(const size_t threads)
            : ThreadPool(threads, 0)
        { }

        ThreadPool(const size_t threads, const size_t maxWork)
            : queue_(maxWork), threads_(threads)
        {
            resize(threads);
        }

        ~ThreadPool()
        {
            stop();
        }

        /** Stops the thread pool, clears its work queue and waits for its threads to join. */
        void stop()
        {
            // send stop signal to each thread
            for(auto thread : threads_)
                thread->stop();

            // push some dummy work for each worker thread
            for(size_t i = 0; i < threads_.size(); ++i)
                run([](){});

            // wait for threads to join
            for(auto thread : threads_)
                thread->join();

            // clear the task queue
            clear();
        }

        /** Schedules the given functor for being executed within the thread pool.
          * @param func functor which shall be executed on the thread pool
          * @return work item which was created and scheduled */
        template<typename T>
        Work::Ptr run(const T &func)
        {
            auto work = std::make_shared<Work>([func](){ func(); });
            work->setState(Work::State::Waiting);
            queue_.enqueue(work);
            return work;
        }

        /** Clears the current work queue. */
        void clear()
        {
            queue_.clear([](Work::Ptr work) { work->setState(Work::State::Cancelled); });
        }

        /** Number of threads running within the thread pool.
          * @return number of threads */
        size_t threads() const
        {
            return threads_.size();
        }

        template<typename Func, typename DataList>
        void foreach(const Func& func, DataList &data)
        {
            std::vector<Work::Ptr> works;

            for(auto &d : data)
            {
                auto work = run([func, &d]() { func(d); });
                works.push_back(work);
            }

            waitAll(works);
        }
    };
}


#endif