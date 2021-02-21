# threadpool-cpp

![Cpp11](https://img.shields.io/badge/C%2B%2B-11-blue.svg)
![License](https://img.shields.io/packagist/l/doctrine/orm.svg)
![CMake](https://github.com/Rookfighter/threadpool-cpp/actions/workflows/cmake.yml/badge.svg)

`threadpool-cpp` is a single header-only C++ library implementing thread pools for C++11.

## Install

Simply copy the header file into your project or install it using
the CMake build system by typing

```bash
cd path/to/repo
mkdir build
cd build
cmake ..
make install
```

## Usage

The main step to use `threadpool-cpp` is to decide on how many threads you want to use
and then create a thread pool.

Then you can start scheduling work functors using the `ThreadPool::run()` method. The method
returns the work item that was scheduled in the work queue. You can use that work item to
wait for its completion.

There are also convenience functions for manipulating lists in parallel and waiting
for lists of tasks.

The following snippet shows a simple example on how to create a thread pool and handle work items.
Also have a look at the `examples/` directory for further examples.

```cpp
#include <iostream>
#include <threadpool.h>

int main()
{
    // Start a new thread pool. This automatically starts its worker threads.
    // Specify the amount of threads in the constructor.
    // Omit the parameter to let the thread pool automatically detect a
    // suitable number.
    tpcpp::ThreadPool pool(4);

    std::cout << "Using " << pool.threads() << " threads" << std::endl;

    std::vector<tpcpp::Work::Ptr> workList(25);
    // run some work items
    for(size_t i = 0; i < workList.size(); ++i)
    {
        // the run() method expects a functor with no return value and parameters
        // it returns the created work item
        // the work item can be used to track the execution state, like Completed, Errored, etc.
        workList[i] = pool.run([i](){std::cout << "I got number " << i << std::endl;});
    }

    // wait for all tasks in the list to finish
    tpcpp::waitAll(workList);

    // create some data to operate on
    std::vector<double> data = {1, 2, 3, 4, 5, 6, 7, 8};

    // create a function that operates on the data and changes it in-place
    auto func = [](double &val){val *= val;};

    // execute the function on each element in parallel
    // foreach waits until the created tasks have finished
    pool.foreach(func, data);

    for(size_t i = 0; i < data.size(); ++i)
        std::cout << data[i] << ", ";

    std::cout << std::endl;

    return 0;
}
```
