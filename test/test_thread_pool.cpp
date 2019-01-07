/*
 * test_thread_pool.cpp
 *
 *  Created on: 28 Aug 2018
 *      Author: Fabian Meyer
 */

#include <catch.hpp>
#include "tpool/thread_pool.h"

using namespace tpool;

TEST_CASE("thread pool")
{
    ThreadPool pool(4);
    REQUIRE(pool.size() == 4);

    SECTION("execute tasks")
    {
        std::mutex mutex;
        unsigned int actCnt = 0;
        unsigned int expCnt = 4;

        for(size_t i = 0; i < expCnt; ++i)
            pool.enqueue([&actCnt, &mutex](){std::unique_lock<std::mutex>lock(mutex); ++actCnt;});
        pool.wait();

        REQUIRE(expCnt == actCnt);
    }

    SECTION("foreach list")
    {
        std::vector<double> data(8, 0.0);
        for(size_t i = 0; i < data.size(); ++i)
            data[i] = static_cast<double>(i);

        auto func = [](double &n){n *= n;};

        tpool::foreach<double>(pool, func, data);

        for(size_t i = 0; i < data.size(); ++i)
            REQUIRE(data[i] == static_cast<double>(i * i));
    }

    SECTION("foreach index")
    {
        std::vector<double> data(8, 0.0);
        for(size_t i = 0; i < data.size(); ++i)
            data[i] = static_cast<double>(i);

        auto func = [&data](const size_t i){data[i] *= data[i];};

        tpool::forindex<size_t>(pool, func, data.size());

        for(size_t i = 0; i < data.size(); ++i)
            REQUIRE(data[i] == static_cast<double>(i * i));
    }
}
