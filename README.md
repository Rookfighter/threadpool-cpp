# threadpool-cpp

![Cpp11](https://img.shields.io/badge/C%2B%2B-11-blue.svg)
![License](https://img.shields.io/packagist/l/doctrine/orm.svg)

threadpool-cpp is a single header-only C++ library implementing thread pools for C++11.

## Install

Simply copy the header file into your project or install them using
the CMake build system by typing

```bash
cd path/to/repo
mkdir build
cd build
cmake ..
make install
```

## Usage

```cpp
#include <iostream>
#include <tpool/thread_pool.h>

int main()
{
    // start a thread pool
    // constructor allows to specify the amount of threads
    tpool::ThreadPool pool(4);

    // enqueue some tasks
    for(size_t i = 0; i < 4; ++i)
        pool.enqueue([i](){std::cout << "I got number " << i << std::endl;});
    // wait until all tasks have been processed
    pool.wait();

    // create some data to operate on
    std::vector<double> data = {1, 2, 3, 4, 5, 6, 7, 8};
    // create a function that operates on the data and changes it in-place
    auto func = [](double &val){val *= val;};
    // execute the function on each element in parallel
    // foreach contains an implicit wait() call to ThreadPool
    tpool::foreach<double>(pool, func, data);

    // we can do the same thing using indices
    auto func_idx = [&data](const size_t i){data[i] *= data[i];};
    tpool::forindex<size_t>(pool, func_idx, data.size());

    return 0;
}
```
