/*
 * ---------------------------------------------------
 * ThreadPool.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/ThreadPool.hpp"

#include <cassert>
#include <common/TracySystem.hpp>
#include <format>

#include <tracy/Tracy.hpp>

namespace GE
{

ThreadPool::ThreadPool(std::size_t workerCount)
{
    assert(workerCount > 0);

    try {
        m_workers.reserve(workerCount);
        for (std::size_t i = 0; i < workerCount; i++)
            m_workers.emplace_back([this, i] {
                tracy::SetThreadNameWithHint(std::format("pool_worker_{}", i).c_str(), 1);
                workerLoop();
            });
    } catch (...) {
        {
            std::lock_guard lock(m_mutex);
            m_stopping = true;
        }
        m_taskAvailable.notify_all();
        for (std::thread& worker : m_workers) {
            if (worker.joinable())
                worker.join();
        }
        throw;
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::lock_guard lock(m_mutex);
        m_stopping = true;
    }
    m_taskAvailable.notify_all();

    for (std::thread& worker : m_workers) {
        if (worker.joinable())
            worker.join();
    }
}

void ThreadPool::workerLoop()
{
    while (true) {
        std::packaged_task<void()> task;
        {
            std::unique_lock lock(m_mutex);
            m_taskAvailable.wait(lock, [this] {
                return m_stopping || m_tasks.empty() == false;
            });
            if (m_stopping && m_tasks.empty())
                return;

            task = std::move(m_tasks.front());
            m_tasks.pop();
        }

        task();
    }
}

} // namespace GE
