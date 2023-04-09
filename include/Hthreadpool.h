#pragma once
#include "noncopyable.h"
#include <vector>
#include <queue>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <functional>
#include <thread>
#include <unordered_map>
#include <future>
#include <chrono>

const int TASK_MAX_THRESHHOLD = 2;
const int THREAD_MAX_THRESHHOLD = 200;
const int Thread_Max_IDLE_TIME = 60;
enum class PoolMode
{
    MODE_FIXED,
    MODE_CACHED,
};

class Thread
{
public:
    using ThreadFunc = std::function<void(int)>;

    Thread(ThreadFunc func)
        : func_(func), threadId_(generateId_++)
    {
    }

    ~Thread() = default;

    void start()
    {
        // 创建一个一个线程对象，并执行一个线程函数
        std::thread t(func_, threadId_);
        t.detach(); // 设置分离线程
    }

    int getThreadId_() const
    {
        return threadId_;
    }

private:
    ThreadFunc func_;
    static int generateId_;
    int threadId_;
};
int Thread::generateId_ = 0;
class ThreadPool : noncopyable
{
public:
    ThreadPool()
        : initThreadSize_(0), taskSize_(0), taskQueMaxThreshHold_(TASK_MAX_THRESHHOLD), mode_(PoolMode::MODE_FIXED), isPoolRunning_(false), idleThreadSize_(0), threadSizeThreshHold_(THREAD_MAX_THRESHHOLD), curThreadSize_(0)
    {
    }

    ~ThreadPool()
    {
        isPoolRunning_ = false;

        std::unique_lock<std::mutex> lock(taskQueMtx_);
        notEmpty_.notify_all();
        exitcond_.wait(lock, [&]() -> bool
                       { return threads_.size() == 0; });
    }

    void setThreadMaxThreshHold(int threadMaxThreshHold)
    {
        if (CheckRunningState())
            return;
        if (mode_ == PoolMode::MODE_CACHED)
            threadSizeThreshHold_ = threadMaxThreshHold;
    }
    void start(size_t initThreadSize = std::thread::hardware_concurrency())
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
    void setTaskQueMaxThreshHold(size_t taskQueMaxThreshHold)
    {

        if (CheckRunningState())
            return;
        taskQueMaxThreshHold_ = taskQueMaxThreshHold;
    }
    //使用可变参模板
    template<typename Func, typename... Args>
    auto submitTask(Func&& func, Args&&... args) -> std::future<decltype(func(args...))>
    {
        using RType = decltype(func(args...));
        auto task = std::make_shared<std::packaged_task<RType()>>(std::bind(std::forward<Func>(func),std::forward<Args>(args)...));
        std::future<RType> result = task->get_future();

    // 获取锁
    std::unique_lock<std::mutex> lock(taskQueMtx_);
    // 线程通信 等待任务队列有空余
    // 用户提交任务，最长不能阻塞超过1s,否则判断任务提交失败，返回 wait wait_for wait_until
    if (!notFull_.wait_for(lock, std::chrono::seconds(1), [&]() -> bool
                           { return taskQue_.size() < taskQueMaxThreshHold_; }))
    {
        // 表示notfull等待一秒钟，条件还是没有满足
        std::cerr << "taskQue is full , submit task fail " << std::endl;
        auto task = std::make_shared<std::packaged_task<RType()>>([]()->RType { return RType();});
        (*task)();
        return task->get_future(); 
    }

    taskQue_.emplace([task](){(*task)();});
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
        // std::cout << "线程 + 1" << std::endl;
    }

    return result; // 理解对象的生存周期

    }
    void setMode(PoolMode mode) 
    {
        if (CheckRunningState())
            return;
        mode_ = mode;
    }

private:
    // 定义线程函数
    void threadFunc(int threadId)
    {
        auto lastTime = std::chrono::high_resolution_clock().now();
        for (;;)
        {
            Task task;
            {
                //std::this_thread::sleep_for(std::chrono::milliseconds(10));
                // 获取锁
                // std::cout << "tid: " << std::this_thread::get_id()
                //  << " 尝试获取任务... " << std::endl;
                std::unique_lock<std::mutex> lock(taskQueMtx_);

                // cache模式下，有可能创建了很多的任务？到某个线程空闲了超过60s时，我们
                // 应该回收多余的线程。

                while (taskQue_.size() == 0) // ！！！！锁加双重判断
                {
                    if (!isPoolRunning_)
                    {
                        threads_.erase(threadId);
                        exitcond_.notify_all();
                        std::cout << "thread exit" << std::endl;
                        return;
                    }
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
                                // std::cout << "线程 - 1" << std::endl;
                                return;
                            }
                        }
                    }
                    else
                    {
                        notEmpty_.wait(lock);
                    }
                    // //线程池要结束
                    // if(!isPoolRunning_)
                    // {
                    //     threads_.erase(threadId);
                    //     std::cout << "thread exit" << std::endl;
                    //     exitcond_.notify_one();
                    //     return;
                    // }
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
                task(); // 放在锁外面，减少锁粒度
            }
            idleThreadSize_++;
            lastTime = std::chrono::high_resolution_clock().now();
        }
    }
    // 检查线程池的运行状态
    bool CheckRunningState() const
    {
        return isPoolRunning_;
    }

private:
    using Task = std::function<void()>;
    // std::vector<std::unique_ptr<Thread>> threads_; // 线程列表
    std::unordered_map<int, std::unique_ptr<Thread>> threads_; // 线程列表
    size_t initThreadSize_;                                    // 初始线程数量
    int threadSizeThreshHold_;                                 // 线程的上限阈值
    std::atomic_int idleThreadSize_;                           // 空闲线程的数量
    std::atomic_int curThreadSize_;                            // 当前线程个数

    std::queue<Task> taskQue_; // 任务队列
    std::atomic_uint taskSize_;                 // 任务个数
    size_t taskQueMaxThreshHold_;               // 任务队列数量上线阈值
    std::mutex taskQueMtx_;                     // 保证任务队列线程安全

    std::condition_variable notFull_;  // 任务队列不满
    std::condition_variable notEmpty_; // 任务队列不空
    std::condition_variable exitcond_; // 等待子线程资源全部回收
    PoolMode mode_;                    // 当前线程池的工作模式
    std::atomic_bool isPoolRunning_;   // 线程池是否已经启动
};
