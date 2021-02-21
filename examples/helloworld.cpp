


#include <threadpool.h>
#include <iostream>


int main()
{
    tpcpp::ThreadPool pool(4);
    std::vector<tpcpp::Work::Ptr> works(10000);

    for(size_t i = 0; i < works.size(); ++i)
    {
        works[i] = pool.run([i]() { std::cout << "Work " << i << std::endl; });
    }

    return 0;
}