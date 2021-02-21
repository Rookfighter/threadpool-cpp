#include <threadpool.h>
#include <iostream>
#include <random>

int main()
{
    // create a random number generator to create random wait times
    std::default_random_engine generator;
    std::uniform_int_distribution<size_t> distribution(500, 2500);

    // create a thread pool with optimal thread count
    tpcpp::ThreadPool pool;

    // print the amount of threads that the pool uses
    std::cout << "Threads: " << pool.threads() << std::endl;

    // create a list to keep track of the tasks
    std::vector<tpcpp::Work::Ptr> works(100);

    // keep track of the theoretically accumulated time
    size_t timeSum = 0;
    for(size_t i = 0; i < works.size(); ++i)
    {
        // create some random sleep time in milliseconds
        auto sleepTime = distribution(generator);
        timeSum += sleepTime;

        // schedule work that sleeps for some time and
        // then prints its number
        works[i] = pool.run(
            [i, sleepTime]()
            {
                std::this_thread::sleep_for (std::chrono::milliseconds(sleepTime));
                std::cout << "Work " << i << std::endl;
            });
    }

    // get the start time stamp
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    // wait for all tasks to finish
    tpcpp::waitAll(works);
    // get the end time stamp
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "Sum of times: " << timeSum << "ms" << std::endl;
    std::cout << "Measured time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;

    return 0;
}