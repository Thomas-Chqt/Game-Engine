/*
 * ---------------------------------------------------
 * ThreadPool_testCases.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include <gtest/gtest.h>

#include "Game-Engine/ThreadPool.hpp"

#include <atomic>
#include <future>
#include <stdexcept>

namespace GE_tests
{

TEST(ThreadPoolTest, submittedTaskReturnsValue)
{
    GE::ThreadPool threadPool(1);

    std::future<int> future = threadPool.submit([] { return 42; });

    EXPECT_EQ(future.get(), 42);
}

TEST(ThreadPoolTest, submittedTaskPropagatesException)
{
    GE::ThreadPool threadPool(1);

    std::future<void> future = threadPool.submit([] {
        throw std::runtime_error("thread pool test exception");
    });

    EXPECT_THROW(future.get(), std::runtime_error);
}

TEST(ThreadPoolTest, multipleSubmittedTasksComplete)
{
    GE::ThreadPool threadPool(2);

    std::future<int> first = threadPool.submit([] { return 1; });
    std::future<int> second = threadPool.submit([] { return 2; });
    std::future<int> third = threadPool.submit([] { return 3; });

    EXPECT_EQ(first.get() + second.get() + third.get(), 6);
}

TEST(ThreadPoolTest, destructorDrainsSubmittedTasks)
{
    std::atomic_uint completedTasks = 0;

    {
        GE::ThreadPool threadPool(1);
        threadPool.submit([&] { completedTasks.fetch_add(1); });
        threadPool.submit([&] { completedTasks.fetch_add(1); });
        threadPool.submit([&] { completedTasks.fetch_add(1); });
    }

    EXPECT_EQ(completedTasks.load(), 3u);
}

} // namespace GE_tests
