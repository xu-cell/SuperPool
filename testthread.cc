#include <iostream>
#include "./include/Hthreadpool.h"
#include <chrono>
#include <thread>
using namespace std;
// class Mytask : public Task
// {
// public:
//     // 如何设计run的返回值类型，可以表示任意类型
//     // c++17->Any类型
//     Any run()
//     {
//         std::cout << "tid: " << std::this_thread::get_id()
//                   << " Task beagin run " << std::endl;
//         std::this_thread::sleep_for(std::chrono::milliseconds(10));
//         std::cout << "tid: " << std::this_thread::get_id()
//                   << " Task end run " << std::endl;
//                   return 1;
//     }
// };
int sum(int a,int b)
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return a + b;
}
int sum2(int a,int b,int c)
{
    std::this_thread::sleep_for(std::chrono::seconds(3));
    return a + b + c;
}
int main()
{
    {
     ThreadPool pool;
     //pool.setMode(PoolMode::MODE_CACHED);
     pool.start(2);
     future<int>res1 =  pool.submitTask(sum,10,20);
     future<int>res2 =  pool.submitTask(sum,10,20);
     future<int>res3 =  pool.submitTask(sum2,10,20,10);
     future<int>res4 =  pool.submitTask(sum2,10,20,5);
     future<int>res5 =  pool.submitTask(sum2,10,20,100);
     int num5 = res5.get();
     cout << num5 << endl;
     int sum = res1.get() + res2.get() + res3.get() + res4.get()  + num5 ;
     cout << sum << endl;
    //  std::this_thread::sleep_for(std::chrono::seconds(10));
    // //
    //     ThreadPool pool;
    //     pool.setMode(PoolMode::MODE_CACHED);
    //     pool.start(2);
    //     // 如何设置result机制
    //     pool.submitTask(std::make_shared<Mytask>());
    //     pool.submitTask(std::make_shared<Mytask>());
    //     pool.submitTask(std::make_shared<Mytask>());
    //     pool.submitTask(std::make_shared<Mytask>());
    //     pool.submitTask(std::make_shared<Mytask>());
    //     // std::this_thread::sleep_for(std::chrono::seconds(5));
    //     std::this_thread::sleep_for(std::chrono::seconds(20));
        
    // }
    std::this_thread::sleep_for(std::chrono::seconds(10));
    // std::cout << "结束" << std::endl;
    }
   
     cout << "main over" << endl;
   
}