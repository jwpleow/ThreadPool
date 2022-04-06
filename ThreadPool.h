#pragma once

#include <array>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <stdexcept>
#include <type_traits>
#include <functional>

template<std::size_t NumThreads>
class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool();

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::invoke_result<F, Args...>::type>;
private:
    // need to keep track of threads so we can join them
    std::array< std::thread, NumThreads > workers;
    // the task queue
    std::queue< std::packaged_task<void()> > tasks;
    
    // synchronization
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop{false};
};
 
// the constructor just launches some amount of workers
template<std::size_t NumThreads>
inline ThreadPool<NumThreads>::ThreadPool()
{
    for(size_t i = 0; i < NumThreads; ++i)
        workers[i] = std::thread(
            [this]
            {
                for(;;)
                {
                    std::packaged_task<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock,
                            [this]{ return this->stop || !this->tasks.empty(); });
                        if(this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    task();
                }
            }
        );
}

// add new work item to the pool
template<std::size_t NumThreads>
template<class F, class... Args>
auto ThreadPool<NumThreads>::enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::invoke_result<F, Args...>::type>
{
    using return_type = typename std::invoke_result<F, Args...>::type;

    std::packaged_task<return_type()> task(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
    std::future<return_type> res = task.get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace(std::move(task));
    }
    condition.notify_one();
    return res;
}

// the destructor joins all threads
template<std::size_t NumThreads>
inline ThreadPool<NumThreads>::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(std::thread &worker: workers)
        worker.join();
}
