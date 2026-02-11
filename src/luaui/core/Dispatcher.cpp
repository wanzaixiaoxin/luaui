#include "Dispatcher.h"
#include <chrono>
#include <algorithm>

namespace luaui {

thread_local Dispatcher* Dispatcher::s_currentDispatcher = nullptr;

Dispatcher::Dispatcher() {
}

Dispatcher::~Dispatcher() {
    Shutdown();
}

void Dispatcher::Initialize(HWND hwnd) {
    m_threadId = GetCurrentThreadId();
    m_messageWindow = hwnd;
    m_running = true;
    s_currentDispatcher = this;
}

void Dispatcher::Shutdown() {
    m_running = false;
    
    // 清空任务队列
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        std::priority_queue<Task> empty;
        m_taskQueue.swap(empty);
    }
    
    s_currentDispatcher = nullptr;
}

void Dispatcher::BeginInvoke(Action action, DispatcherPriority priority) {
    if (!m_running) return;
    
    Task task{
        std::move(action),
        priority,
        GetTickCount64()
    };
    
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_taskQueue.push(std::move(task));
    }
    
    // 通知UI线程
    if (!CheckAccess()) {
        PostMessageToUIThread();
    }
}

bool Dispatcher::Invoke(Action action, DispatcherPriority priority) {
    // 如果在UI线程，直接执行
    if (CheckAccess()) {
        action();
        return true;
    }
    
    // 异步转同步
    std::atomic<bool> completed{false};
    std::exception_ptr exception;
    
    BeginInvoke([&]() {
        try {
            action();
        } catch (...) {
            exception = std::current_exception();
        }
        completed = true;
    }, priority);
    
    // 等待完成
    while (!completed) {
        Sleep(1); // 简单自旋等待，生产环境可用条件变量优化
    }
    
    if (exception) {
        std::rethrow_exception(exception);
    }
    
    return true;
}

bool Dispatcher::Invoke(Action action, std::chrono::milliseconds timeout) {
    if (CheckAccess()) {
        action();
        return true;
    }
    
    auto start = std::chrono::steady_clock::now();
    std::atomic<bool> completed{false};
    
    BeginInvoke([&]() {
        action();
        completed = true;
    });
    
    // 带超时的等待
    while (!completed) {
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (elapsed > timeout) {
            return false; // 超时
        }
        Sleep(1);
    }
    
    return true;
}

bool Dispatcher::ProcessOneTask() {
    VerifyAccess(); // 必须在UI线程调用
    
    Task task;
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        if (m_taskQueue.empty()) return false;
        
        task = m_taskQueue.top();
        m_taskQueue.pop();
    }
    
    ExecuteTask(task);
    return true;
}

size_t Dispatcher::ProcessAllTasks(uint32_t maxTimeMs) {
    VerifyAccess();
    
    auto start = GetTickCount();
    size_t count = 0;
    
    while (m_running) {
        // 检查时间预算
        if (GetTickCount() - start > maxTimeMs) break;
        
        if (!ProcessOneTask()) break;
        count++;
    }
    
    return count;
}

LRESULT Dispatcher::ProcessMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_DISPATCHER_INVOKE) {
        // 处理所有待处理任务
        if (auto* disp = Current()) {
            disp->ProcessAllTasks(16); // 最多16ms，保证帧率
        }
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

Dispatcher* Dispatcher::Current() {
    return s_currentDispatcher;
}

Dispatcher::Stats Dispatcher::GetStats() const {
    Stats stats;
    stats.processedCount = m_processedCount.load();
    stats.avgQueueTimeUs = m_processedCount > 0 
        ? static_cast<double>(m_totalQueueTime.load()) / m_processedCount 
        : 0;
    
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_queueMutex));
    stats.pendingCount = m_taskQueue.size();
    
    return stats;
}

void Dispatcher::PostMessageToUIThread() {
    if (m_messageWindow && IsWindow(m_messageWindow)) {
        PostMessage(m_messageWindow, WM_DISPATCHER_INVOKE, 0, 0);
    }
}

void Dispatcher::ExecuteTask(const Task& task) {
    auto queueTime = GetTickCount64() - task.timestamp;
    m_totalQueueTime += queueTime * 1000; // 转为微秒
    m_processedCount++;
    
    if (task.action) {
        task.action();
    }
}

} // namespace luaui
