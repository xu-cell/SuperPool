#pragma once
#include "noncopyable.h"
#include <vector>
#include<queue>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <functional>
#include <thread>
enum class PollMode
{
    MODE_FIXED,
    MODE_CACHED,
};
class Thread
{
public:
    using ThreadFunc = std::function<void()>;
   
    Thread(ThreadFunc func); 
   
    ~Thread();
    
    void start();
private:
    ThreadFunc func_;

};

class Task
{
public:
    virtual void run() = 0;
};

class ThreadPool : noncopyable
{
public:
    ThreadPool();
    ~ThreadPool();
    void start(size_t initThreadSize = 6);
    void setTaskQueMaxThreshHold(int taskQueMaxThreshHold);
    void submitTask(std::shared_ptr<Task>sp);
    void setMode(PollMode mode);
private:
    //定义线程函数
    void threadFunc();
private: 
    std::vector<Thread*> threads_;  //线程列表
    size_t initThreadSize_; //初始线程数量
    std::queue<std::shared_ptr<Task>>taskQue_;//任务队列
    std::atomic_uint taskSize_; // 任务个数
    int taskQueMaxThreshHold_; //任务队列数量上线阈值
    std::mutex taskQueMtx_; //保证任务队列线程安全
    std::condition_variable notFull_;  //任务队列不满
    std::condition_variable notEmpty_; //任务队列不空
    PollMode mode_; //当前线程池的工作模式
};