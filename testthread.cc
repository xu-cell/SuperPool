#include <iostream>
#include "./include/threadpool.h"
#include <chrono>
#include <thread>

class Mytask : public Task
{
public:
    // 如何设计run的返回值类型，可以表示任意类型
    // c++17->Any类型
    Any run()
    {
        std::cout << "tid: " << std::this_thread::get_id()
                  << " Task beagin run " << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        std::cout << "tid: " << std::this_thread::get_id()
                  << " Task end run " << std::endl;
    }
};

int main()
{
    {
        ThreadPool pool;
        pool.setMode(PoolMode::MODE_CACHED);
        pool.start(2);
        // 如何设置result机制
        pool.submitTask(std::make_shared<Mytask>());
        pool.submitTask(std::make_shared<Mytask>());
        pool.submitTask(std::make_shared<Mytask>());
        pool.submitTask(std::make_shared<Mytask>());
        pool.submitTask(std::make_shared<Mytask>());
        // std::this_thread::sleep_for(std::chrono::seconds(5));
        std::this_thread::sleep_for(std::chrono::seconds(20));
        
    }
      std::this_thread::sleep_for(std::chrono::seconds(10));
    std::cout << "结束" << std::endl;
   
    return 0;
}