// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef LSPSERVER_THRDPOOL_H
#define LSPSERVER_THRDPOOL_H

#include <cstddef>
#include <functional>
#include <memory>
#include <queue>
#include <thread>
#include <unordered_set>
#ifdef __APPLE__
#include <pthread.h>
#include "common/Constants.h"
#endif

namespace ark {
/**
 * @class ThrdPool
 * @brief A thread pool for task execution, supporting synchronous or asynchronous task execution
 *
 */
class ThrdPool {
public:
    struct Task {
        std::function<void()> task;
        uint64_t id;
        bool running = false;
    };
#ifdef __APPLE__
    explicit ThrdPool(const size_t threads) : stop(false)
    {
        for (size_t i = 0; i < threads; ++i) {
            auto wrapper = new TaskWrapper(this);
            pthread_t thread;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setstacksize(&attr, CONSTANTS::MAC_THREAD_STACK_SIZE);

            int result = pthread_create(&thread, &attr, ThreadRoutine, wrapper);
            if (result != 0) {
                Trace::Elog("Failed to create thread");
            }

            pthread_attr_destroy(&attr);
            workers.emplace_back(thread);
        }
    }
#else
    explicit ThrdPool(const size_t threads) : stop(false)
    {
        for (size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] {
                for (;;) {
                    std::shared_ptr<Task> task = nullptr;
                    {
                        std::unique_lock<std::mutex> lock(this->queueMu);
                        this->cv.wait(lock, [this] { return this->stop || !this->readyTasks.empty(); });
                        if (this->stop && this->readyTasks.empty()) {
                            return;
                        }
                        task = this->readyTasks.front();
                        task->running = true;
                        this->readyTasks.pop();
                    }
                    task->task();
                }
            });
        }
    }
#endif

    template <class F, class... Args>
    void AddTask(const uint64_t taskId, const std::unordered_set<uint64_t> &dependencies, F &&func, Args &&...args)
    {
        std::shared_ptr<Task> task = std::make_unique<Task>(Task{std::bind(std::forward<F>(func)), taskId});
        if (!task) {
            return;
        }

        {
            std::unique_lock<std::mutex> lock(queueMu);
            if (taskMap.find(taskId) == taskMap.end()) {
                 ++tasksRemaining; // Increment the count of remaining tasks
            }
            taskMap[taskId] = task;

            for (uint64_t dep : dependencies) {
                taskDependencies[taskId].insert(dep);
                dependentTasks[dep].insert(taskId);
            }

            if (taskDependencies[taskId].empty()) {
                readyTasks.push(task);
                cv.notify_one();
            }
        }
    }

    void TaskCompleted(const uint64_t taskId, bool inPool = true)
    {
        std::unique_lock<std::mutex> lock(queueMu);
        if (taskMap.find(taskId) == taskMap.end() || (taskMap[taskId] && !taskMap[taskId]->running)) {
            return;
        }

        for (uint64_t dep : dependentTasks[taskId]) {
            taskDependencies[dep].erase(taskId);
            if (taskDependencies[dep].empty()) {
                readyTasks.push(taskMap[dep]);
                cv.notify_one();
            }
        }

        // Clean up the completed task
        dependentTasks.erase(taskId);
        taskDependencies.erase(taskId);
        taskMap.erase(taskId);

        if (inPool && --tasksRemaining == 0) {
            completionCv.notify_all(); // Ensure to notify when all tasks complete
        }
    }

    void WaitUntilAllTasksComplete()
    {
        std::unique_lock<std::mutex> lock(queueMu);
        completionCv.wait(lock, [this] { return tasksRemaining == 0; });
    }

    ~ThrdPool()
    {
        {
            std::unique_lock<std::mutex> lock(queueMu);
            stop = true;
        }
        cv.notify_all();
#ifdef __APPLE__
        for (pthread_t &worker : workers) {
            pthread_join(worker, NULL);
        }
#else
        for (std::thread &worker : workers) {
            worker.join();
        }
#endif
    }

private:
#ifdef __APPLE__
    class TaskWrapper {
    public:
        explicit TaskWrapper(ThrdPool *pool) : pool_(pool) {}
        void operator()()
        {
            for (;;) {
                std::shared_ptr<Task> task = nullptr;
                {
                    std::unique_lock<std::mutex> lock(pool_->queueMu);
                    pool_->cv.wait(lock, [this] { return pool_->stop || !pool_->readyTasks.empty(); });
                    if (pool_->stop && pool_->readyTasks.empty()) {
                        return;
                    }
                    task = pool_->readyTasks.front();
                    task->running = true;
                    pool_->readyTasks.pop();
                }
                task->task();
            }
        }

    private:
        ThrdPool *pool_;
    };

    static void *ThreadRoutine(void *arg)
    {
        TaskWrapper *taskWrapper = static_cast<TaskWrapper *>(arg);
        (*taskWrapper)();
        delete taskWrapper;
        return nullptr;
    }

    std::vector<pthread_t> workers;
#else
    std::vector<std::thread> workers;
#endif
    std::unordered_map<uint64_t, std::shared_ptr<Task>> taskMap;
    std::unordered_map<uint64_t, std::unordered_set<uint64_t>> taskDependencies;
    std::unordered_map<uint64_t, std::unordered_set<uint64_t>> dependentTasks;
    std::queue<std::shared_ptr<Task>> readyTasks;
    std::mutex queueMu;
    std::condition_variable cv;
    std::condition_variable completionCv;
    bool stop;
    size_t tasksRemaining = 0;
};
} // namespace ark

#endif // LSPSERVER_THRDPOOL_H
