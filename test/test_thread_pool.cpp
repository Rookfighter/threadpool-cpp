/*
 * test_thread_pool.cpp
 *
 *  Created on: 28 Aug 2018
 *      Author: Fabian Meyer
 */

#include <catch.hpp>
#include "tpcpp/thread_pool.h"

using namespace tp;

TEST_CASE("thread pool")
{
    ThreadPool pool(4);
    REQUIRE(pool.size() == 4);

    SECTION("execute tasks")
    {
        std::mutex mutex;
        unsigned int actCnt = 0;
        unsigned int expCnt = 4;

        for(unsigned int i = 0; i < expCnt; ++i)
            pool.enqueue([&actCnt, &mutex](){std::unique_lock<std::mutex>(mutex); ++actCnt;});
        pool.wait();

        REQUIRE(expCnt == actCnt);
    }
}
