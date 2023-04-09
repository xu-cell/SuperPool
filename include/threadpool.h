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
enum class PoolMode
{
    MODE_FIXED,
    MODE_CACHED,
};
// Any类型，可以接受任意数据类型---实际是里面的base类
class Any
{
public:
    Any() = default;
    ~Any() = default;
    Any(const Any&) = delete;
    Any& operator=(const Any&) = delete;
    Any(Any&&) = default;
    Any& operator=(Any&&) = default;

    template<typename T>
    Any(T data):base_(std::make_unique<Derive<T>>(data)){}

    template<typename T>
    T cast_()
    {
        Derive<T> *pd = dynamic_cast<Derive<T>*>(base_.get());
        if(pd == nullptr)
        {
            throw "type is unmatch!";
        }
        return pd->data_;
    }

private:
    // 基类类型
    class Base
    {
    public:
        virtual ~Base() = default;
    };

    // 派生类类型

    template <typename T>
    class Derive : public Base
    {
    public:
        Derive(T data) : data_(data) {}
        T data_;
    };

private:
    std::unique_ptr<Base> base_;
};

class Thread
{
public:
    using ThreadFunc = std::function<void(int)>;

    Thread(ThreadFunc func);

    ~Thread();

    void start();

    int getThreadId_() const;

private:
    ThreadFunc func_;
    static int generateId_;
    int threadId_;
};
class Result;
class Task
{
public:
    Task();
    ~Task() = default;

    void exec();
    void setResult(Result* ptr);
    virtual Any run() = 0;
private:
    Result* res_;
};

class Semaphore
{
public:
    Semaphore(int limit = 0) 
        :resLimit_(limit) {}
    ~Semaphore() = default;

    void wait()
    {
        std::unique_lock<std::mutex>lock(mtx_);
        cond_.wait(lock,[&]()->bool {
            return resLimit_ > 0;
        });
        resLimit_--;
    }

    void post()
    {
        std::unique_lock<std::mutex>lock(mtx_);
        resLimit_++;
        cond_.notify_all();
    }
private:
    int resLimit_;
    std::mutex mtx_;
    std::condition_variable cond_;
};
class Result
{
public:
    Result(std::shared_ptr<Task>task,bool isValid = true);

    ~Result() = default;

    void setVal(Any any);

    Any get();

private:
    Any any_;
    Semaphore sem_;
    std::shared_ptr<Task>task_;
    std::atomic_bool isValid_; //表示任务是否提交成功
};


class ThreadPool : noncopyable
{
public:
    ThreadPool();
    ~ThreadPool();
    void setThreadMaxThreshHold(int threadMaxThreshHold);
    void start(size_t initThreadSize =  std::thread::hardware_concurrency());
    void setTaskQueMaxThreshHold(size_t taskQueMaxThreshHold);
    Result submitTask(std::shared_ptr<Task> sp);
    void setMode(PoolMode mode);

private:
    // 定义线程函数
    void threadFunc(int threadId);
    // 检查线程池的运行状态
    bool CheckRunningState() const ;
private:
    //std::vector<std::unique_ptr<Thread>> threads_; // 线程列表
    std::unordered_map<int,std::unique_ptr<Thread>>threads_; // 线程列表
    size_t initThreadSize_;                        // 初始线程数量
    int threadSizeThreshHold_;                     // 线程的上限阈值
    std::atomic_int idleThreadSize_;               //空闲线程的数量
    std::atomic_int curThreadSize_;                //当前线程个数

    std::queue<std::shared_ptr<Task>> taskQue_;    // 任务队列
    std::atomic_uint taskSize_;                    // 任务个数
    size_t taskQueMaxThreshHold_;                  // 任务队列数量上线阈值
    std::mutex taskQueMtx_;                        // 保证任务队列线程安全
   
    std::condition_variable notFull_;              // 任务队列不满
    std::condition_variable notEmpty_;             // 任务队列不空
    std::condition_variable exitcond_;             //等待子线程资源全部回收
    PoolMode mode_;                                // 当前线程池的工作模式
    std::atomic_bool isPoolRunning_;                //线程池是否已经启动

};



