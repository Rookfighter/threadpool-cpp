


#include <threadpool.h>
#include <iostream>
#include <random>

int main()
{
    tpcpp::ThreadPool pool;
    std::vector<tpcpp::Work::Ptr> works(100);

    std::default_random_engine generator;
    std::uniform_int_distribution<size_t> distribution(500, 2500);

    std::cout << "Threads: " << pool.threads() << std::endl;

    size_t sum = 0;
    for(size_t i = 0; i < works.size(); ++i)
    {
        auto sleepTime = distribution(generator);
        sum += sleepTime;
        works[i] = pool.run(
            [i, sleepTime]()
            {
                std::this_thread::sleep_for (std::chrono::milliseconds(sleepTime));
                std::cout << "Work " << i << std::endl;
            });
    }

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    tpcpp::waitAll(works);

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "Sum " << sum << "ms" << std::endl;
    std::cout << "Took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;

    return 0;
}