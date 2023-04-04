#include "../include/threadpool.h"
#include <functional>
#include <iostream>
const int TASH_MAX_THRESHHOLD = 1024;
ThreadPool::ThreadPool()
    : initThreadSize_(0), taskSize_(0), taskQueMaxThreshHold_(TASH_MAX_THRESHHOLD), mode_(PollMode::MODE_FIXED)
{
}
ThreadPool::~ThreadPool()
{
}
void ThreadPool::start(size_t initThreadSize)
{
    initThreadSize_ = initThreadSize;

    for (int i = 0; i < initThreadSize_; i++)
    {
        // 在创建线程的时候给线程提供线程函数
        threads_.emplace_back(new Thread(std::bind(&ThreadPool::threadFunc, this)));
    }
    for (int i = 0; i < initThreadSize_; i++)
    {
        threads_[i]->start();
    }
}
void ThreadPool::setTaskQueMaxThreshHold(int taskQueMaxThreshHold)
{
    taskQueMaxThreshHold_ = taskQueMaxThreshHold;
}
//用户调用接口给线程池提交任务。 --- 生产任务
void ThreadPool::submitTask(std::shared_ptr<Task> sp)
{
}
void ThreadPool::setMode(PollMode mode)
{
    mode_ = mode;
}
 
void Thread::start()
{
    //创建一个一个线程对象，并执行一个线程函数
    std::thread t(func_);
    t.detach(); // 设置分离线程
}
//线程从任务队列里消费任务
void ThreadPool::threadFunc()
{
    std::cout << " begin threadfunc tid : " <<std::this_thread::get_id() << std::endl;
    std::cout << " end threadfunc tid : " <<std::this_thread::get_id() << std::endl;
}
Thread::Thread(ThreadFunc func)
    :func_(func)
{
}

Thread::~Thread()
{

}

