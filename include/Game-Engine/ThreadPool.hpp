/*
 * ---------------------------------------------------
 * ThreadPool.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include "Game-Engine/Export.hpp"

#include <cassert>
#include <condition_variable>
#include <cstddef>
#include <future>
#if defined(_MSC_VER)
#include <memory>
#endif
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace GE
{

class GE_API ThreadPool
{
public:
    ThreadPool() = delete;
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;

    explicit ThreadPool(std::size_t workerCount);

    template<typename Callable>
    auto submit(Callable&& callable) -> std::future<std::invoke_result_t<std::decay_t<Callable>&>>;

    ~ThreadPool();

private:
    void workerLoop();

    std::vector<std::thread> m_workers;
    std::queue<std::packaged_task<void()>> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_taskAvailable;
    bool m_stopping = false;

public:
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;
};

template<typename Callable>
auto ThreadPool::submit(Callable&& callable) -> std::future<std::invoke_result_t<std::decay_t<Callable>&>>
{
    using Task = std::decay_t<Callable>;
    using Result = std::invoke_result_t<Task&>;

#if defined(_MSC_VER)
    auto submittedTask = std::make_shared<std::packaged_task<Result()>>(std::forward<Callable>(callable));
    std::future<Result> future = submittedTask->get_future();
#else
    std::packaged_task<Result()> submittedTask(std::forward<Callable>(callable));
    std::future<Result> future = submittedTask.get_future();
#endif

    {
        std::lock_guard lock(m_mutex);
        assert(m_stopping == false);
        m_tasks.emplace([task = std::move(submittedTask)]() mutable {
#if defined(_MSC_VER)
            (*task)();
#else
            task();
#endif
        });
    }
    m_taskAvailable.notify_one();

    return future;
}

} // namespace GE

#endif // THREADPOOL_HPP
