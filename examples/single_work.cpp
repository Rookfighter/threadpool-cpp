#include <iostream>
#include <threadpool.h>

int main()
{
    // create a thread pool with optimal thread count
    tpcpp::ThreadPool pool;

    std::cout << "Using " << pool.threads() << " threads" << std::endl;

    std:: cout << "Start two work items" << std::endl;

    // create work item 1
    auto work1 = pool.run([]() { std::cout << "I am no 1" << std::endl; });

    // create work item 2
    auto work2 = pool.run([]() { std::cout << "I am no 2" << std::endl; });

    work1->wait();
    work2->wait();

    return 0;
}
