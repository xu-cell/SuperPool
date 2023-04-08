#include "../include/threadpool.h"
#include <functional>
#include <iostream>
#include <chrono>
const int TASK_MAX_THRESHHOLD = 1024;
const int THREAD_MAX_THRESHHOLD = 200;
const int Thread_Max_IDLE_TIME = 10;
ThreadPool::ThreadPool()
    : initThreadSize_(0), taskSize_(0), taskQueMaxThreshHold_(TASK_MAX_THRESHHOLD), mode_(PoolMode::MODE_FIXED), isPoolRunning_(false), idleThreadSize_(0), threadSizeThreshHold_(THREAD_MAX_THRESHHOLD), curThreadSize_(0)
{
}
ThreadPool::~ThreadPool()
{
    isPoolRunning_ = false;
    notEmpty_.notify_all();
    std::unique_lock<std::mutex> lock(taskQueMtx_);
    exitcond_.wait(lock, [&]() -> bool
                   { return threads_.size() == 0; });
}
void ThreadPool::start(size_t initThreadSize)
{
    isPoolRunning_ = true;
    initThreadSize_ = initThreadSize;
    curThreadSize_ = initThreadSize;
    for (int i = 0; i < initThreadSize_; i++)
    {
        // 在创建线程的时候给线程提供线程函数
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1)); // c++14
        int threadid = ptr->getThreadId_();
        threads_.emplace(threadid, std::move(ptr));
        threads_[threadid]->start();
        idleThreadSize_++;
    }
}
void ThreadPool::setTaskQueMaxThreshHold(size_t taskQueMaxThreshHold)
{
    if (CheckRunningState())
        return;
    taskQueMaxThreshHold_ = taskQueMaxThreshHold;
}
Result::Result(std::shared_ptr<Task> task, bool isValid)
    : isValid_(isValid), task_(task)
{
    task_->setResult(this);
}

// 用户调用接口给线程池提交任务。 --- 生产任务
Result ThreadPool::submitTask(std::shared_ptr<Task> sp)
{
    // 获取锁
    std::unique_lock<std::mutex> lock(taskQueMtx_);
    // 线程通信 等待任务队列有空余
    // 用户提交任务，最长不能阻塞超过1s,否则判断任务提交失败，返回 wait wait_for wait_until
    if (!notFull_.wait_for(lock, std::chrono::seconds(1), [&]() -> bool
                           { return taskQue_.size() < taskQueMaxThreshHold_; }))
    {
        // 表示notfull等待一秒钟，条件还是没有满足
        std::cerr << "taskQue is full , submit task fail " << std::endl;
        return Result(sp, false);
    }

    taskQue_.emplace(sp);
    taskSize_++;
    notEmpty_.notify_all();

    // 当任务过多时，根据任务数量和空闲线程数量来判断。我们要合理的增加线程的数量
    if (mode_ == PoolMode::MODE_CACHED && taskSize_ > idleThreadSize_ && curThreadSize_ < threadSizeThreshHold_)
    {
        // std::cout << taskSize_ << "  " << idleThreadSize_ << std::endl;
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1)); // c++14
        int threadid = ptr->getThreadId_();
        threads_.emplace(threadid, std::move(ptr));
        threads_[threadid]->start();
        idleThreadSize_++;
        curThreadSize_++;
        std::cout << "线程 + 1" << std::endl;
    }

    return Result(sp); // 理解对象的生存周期
}
void ThreadPool::setMode(PoolMode mode)
{
    if (CheckRunningState())
        return;
    mode_ = mode;
}

void Thread::start()
{
    // 创建一个一个线程对象，并执行一个线程函数
    std::thread t(func_, threadId_);
    t.detach(); // 设置分离线程
}
// 线程从任务队列里消费任务
void ThreadPool::threadFunc(int threadId)
{
    auto lastTime = std::chrono::high_resolution_clock().now();
    while(isPoolRunning_)
    {
        std::shared_ptr<Task> task;
        {
            // 获取锁
            // std::cout << "tid: " << std::this_thread::get_id()
            //  << " 尝试获取任务... " << std::endl;
            std::unique_lock<std::mutex> lock(taskQueMtx_);

            // cache模式下，有可能创建了很多的任务？到某个线程空闲了超过60s时，我们
            // 应该回收多余的线程。

            while (taskQue_.size() == 0)
            {
                
                if (mode_ == PoolMode::MODE_CACHED)
                {
                    if (std::cv_status::timeout == notEmpty_.wait_for(lock, std::chrono::seconds(1)))
                    {

                        auto now = std::chrono::high_resolution_clock().now();
                        auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
                        if (dur.count() >= Thread_Max_IDLE_TIME && curThreadSize_ > initThreadSize_)
                        {
                            // 回收线程:修改相关变量 + 从容器剔除。
                            curThreadSize_--;
                            idleThreadSize_--;
                            threads_.erase(threadId); //
                            std::cout << "线程 - 1" << std::endl;
                            return;
                        }
                    }
                }
                else
                {
                    notEmpty_.wait(lock);
                }
                //线程池要结束
                if(!isPoolRunning_)
                {
                    threads_.erase(threadId);
                    std::cout << "thread exit" << std::endl;
                    exitcond_.notify_one();
                    return;
                }
            }


            // 有任务来了
            idleThreadSize_--;

            // std::cout << "tid: " << std::this_thread::get_id()
            //       << " 获取任务成功... " << std::endl;
            task = taskQue_.front();
            taskQue_.pop();
            taskSize_--;

            if (taskQue_.size() > 0)
            {
                notEmpty_.notify_all();
            }
        }
        notFull_.notify_all(); // 通知可以生产任务了
        if (task != nullptr)
        {
            task->exec(); // 放在锁外面，减少锁粒度
        }
        idleThreadSize_++;
        lastTime = std::chrono::high_resolution_clock().now();
    }
    threads_.erase(threadId);
    exitcond_.notify_one();
    std::cout << "thread exit" << std::endl;
}
Thread::Thread(ThreadFunc func)
    : func_(func), threadId_(generateId_++)
{
}

Thread::~Thread()
{
}
Any Result::get()
{
    if (!isValid_)
    {
        return "";
    }
    sem_.wait();
    return std::move(any_);
}

void Result::setVal(Any any)
{
    this->any_ = std::move(any);
    sem_.post();
}
void Task::exec()
{
    if (res_ != nullptr)
    {
        res_->setVal(run());
    }
}

void Task::setResult(Result *ptr)
{
    res_ = ptr;
}
Task::Task()
    : res_(nullptr)
{
}

void ThreadPool::setThreadMaxThreshHold(int threadMaxThreshHold)
{
    if (CheckRunningState())
        return;
    if (mode_ == PoolMode::MODE_CACHED)
        threadSizeThreshHold_ = threadMaxThreshHold;
}

int Thread::generateId_ = 0;

int Thread::getThreadId_() const
{
    return threadId_;
}
bool ThreadPool::CheckRunningState() const
{
    return isPoolRunning_;
}