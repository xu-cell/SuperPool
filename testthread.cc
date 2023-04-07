#include <iostream>
#include "./include/threadpool.h"
#include <chrono>
#include <thread>

class Mytask : public Task
{
public:
    //如何设计run的返回值类型，可以表示任意类型
    //c++17->Any类型
    Any run()
    {
        std::cout << "tid: " << std::this_thread::get_id()
                  << " Task beagin run " << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "tid: " << std::this_thread::get_id()
                  << " Task end run " << std::endl;
    }
};

int main()
{
    ThreadPool pool;
    
    pool.start(4);
    //如何设置result机制
    pool.submitTask(std::make_shared<Mytask>());
    pool.submitTask(std::make_shared<Mytask>());
    pool.submitTask(std::make_shared<Mytask>());
    pool.submitTask(std::make_shared<Mytask>());
    pool.submitTask(std::make_shared<Mytask>());
    //std::this_thread::sleep_for(std::chrono::seconds(5));
    getchar();
    return 0;
}