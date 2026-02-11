#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>
#include <atomic>
#include <windows.h>

namespace luaui {

/**
 * @brief UI线程调度器 - 确保UI操作在正确线程执行
 * 
 * 设计原则：
 * - 所有UI控件操作必须在创建它们的线程（UI线程）执行
 * - 后台线程通过Dispatcher.Invoke/BeginInvoke与UI通信
 * - 支持同步等待和异步投递
 * - 与Windows消息循环集成
 */

enum class DispatcherPriority {
    Idle = 1,       // 空闲时处理
    Background = 2, // 低优先级
    Normal = 5,     // 默认
    Render = 7,     // 渲染前
    Input = 8,      // 输入处理
    Loaded = 9,     // 加载完成
    Send = 10       // 立即执行（同步）
};

class Dispatcher {
public:
    using Action = std::function<void()>;
    
    struct Task {
        Action action;
        DispatcherPriority priority;
        uint64_t timestamp;
        
        bool operator<(const Task& other) const {
            // priority_queue默认大顶堆，这里反转实现小顶堆
            return priority < other.priority;
        }
    };

private:
    // 线程标识
    DWORD m_threadId = 0;
    std::atomic<bool> m_running{false};
    
    // 任务队列
    std::priority_queue<Task> m_taskQueue;
    std::mutex m_queueMutex;
    std::condition_variable m_condition;
    
    // 性能统计
    std::atomic<uint64_t> m_processedCount{0};
    std::atomic<uint64_t> m_totalQueueTime{0}; // 微秒
    
    // 与Windows消息集成
    static constexpr UINT WM_DISPATCHER_INVOKE = WM_USER + 0x1001;
    HWND m_messageWindow = nullptr;  // 用于跨线程唤醒消息循环

public:
    Dispatcher();
    ~Dispatcher();

    // 禁止拷贝移动
    Dispatcher(const Dispatcher&) = delete;
    Dispatcher& operator=(const Dispatcher&) = delete;

    /**
     * @brief 初始化调度器（必须在UI线程调用）
     * @param hwnd 关联的窗口句柄，用于跨线程消息通知
     */
    void Initialize(HWND hwnd = nullptr);

    /**
     * @brief 关闭调度器，清空未处理任务
     */
    void Shutdown();

    /**
     * @brief 检查当前是否在UI线程
     */
    bool CheckAccess() const {
        return GetCurrentThreadId() == m_threadId;
    }

    /**
     * @brief 断言在UI线程（调试用）
     */
    void VerifyAccess() const {
#ifdef _DEBUG
        if (!CheckAccess()) {
            throw std::runtime_error("Cross-thread UI access detected!");
        }
#endif
    }

    /**
     * @brief 异步投递任务到UI线程
     * @param action 要执行的动作
     * @param priority 优先级
     */
    void BeginInvoke(Action action, DispatcherPriority priority = DispatcherPriority::Normal);

    /**
     * @brief 同步执行任务（阻塞直到完成）
     * @return 是否成功执行
     * @note 如果当前已在UI线程，直接执行
     */
    bool Invoke(Action action, DispatcherPriority priority = DispatcherPriority::Normal);

    /**
     * @brief 带超时的同步执行
     */
    bool Invoke(Action action, std::chrono::milliseconds timeout);

    /**
     * @brief 处理单个任务（在消息循环中调用）
     * @return 是否处理了任务
     */
    bool ProcessOneTask();

    /**
     * @brief 处理所有待处理任务（用于一帧内批量处理）
     * @param maxTimeMs 最大处理时间（毫秒），防止阻塞
     * @return 处理的任务数量
     */
    size_t ProcessAllTasks(uint32_t maxTimeMs = 16);

    /**
     * @brief 与Windows消息循环集成
     * 在WindowProc中调用此函数处理调度任务
     */
    static LRESULT ProcessMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief 获取当前线程的Dispatcher（线程本地存储）
     */
    static Dispatcher* Current();

    /**
     * @brief 统计信息
     */
    struct Stats {
        uint64_t processedCount;
        uint64_t pendingCount;
        double avgQueueTimeUs;
    };
    Stats GetStats() const;

private:
    void PostMessageToUIThread();
    void ExecuteTask(const Task& task);
    
    // 线程本地存储
    static thread_local Dispatcher* s_currentDispatcher;
};

/**
 * @brief 在析构时自动验证线程访问的辅助类
 */
class UIThreadGuard {
    Dispatcher* m_dispatcher;
public:
    UIThreadGuard() : m_dispatcher(Dispatcher::Current()) {
        if (m_dispatcher) m_dispatcher->VerifyAccess();
    }
};

/**
 * @brief 延迟执行辅助类
 */
class DeferredAction {
    Dispatcher* m_dispatcher;
    Dispatcher::Action m_action;
    
public:
    DeferredAction(Dispatcher::Action action) 
        : m_dispatcher(Dispatcher::Current()), m_action(action) {}
    
    ~DeferredAction() {
        if (m_dispatcher && m_action) {
            m_dispatcher->BeginInvoke(m_action);
        }
    }
    
    // 取消延迟执行
    void Cancel() { m_action = nullptr; }
};

} // namespace luaui
