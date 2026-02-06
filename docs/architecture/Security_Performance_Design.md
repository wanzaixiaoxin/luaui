# 安全与性能优化设计

## 1. 安全架构

### 1.1 安全架构图

```
┌─────────────────────────────────────────────────────────────────┐
│                      安全架构层次                                │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Layer 5: 应用层安全                                             │
│  ├── 输入验证 (XSS/SQL注入防护)                                  │
│  ├── 输出编码                                                   │
│  └── 业务逻辑校验                                               │
│                                                                 │
│  Layer 4: Lua脚本安全                                            │
│  ├── 沙箱执行环境                                                │
│  ├── 内存/CPU限制                                                │
│  ├── API白名单                                                  │
│  └── 文件系统隔离                                               │
│                                                                 │
│  Layer 3: 网络安全                                               │
│  ├── HTTPS强制                                                  │
│  ├── 域名白名单                                                  │
│  ├── 请求签名验证                                                │
│  └── 证书固定                                                   │
│                                                                 │
│  Layer 2: 资源安全                                               │
│  ├── 资源完整性校验                                              │
│  ├── 代码签名验证                                                │
│  └── 反调试/反篡改                                               │
│                                                                 │
│  Layer 1: 系统安全                                               │
│  ├── Windows安全特性                                             │
│  ├── ASLR/DEP                                                   │
│  └── 权限最小化                                                  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 1.2 Lua沙箱安全

```cpp
namespace LuaUI {
namespace Security {

class LuaSandbox {
public:
    struct SecurityPolicy {
        // 资源限制
        size_t maxMemoryMB = 64;
        int maxInstructionsPerSecond = 1000000;
        std::chrono::milliseconds maxExecutionTime = std::chrono::seconds(30);
        int maxOpenFiles = 10;
        int maxNetworkConnections = 5;
        
        // 功能开关
        bool allowFileRead = true;
        bool allowFileWrite = false;
        bool allowNetwork = false;
        bool allowLoadLibrary = false;
        bool allowDebug = false;
        
        // 路径限制
        std::vector<std::string> allowedReadPaths;
        std::vector<std::string> allowedWritePaths;
        std::vector<std::string> blockedPaths;
    };
    
    bool Initialize(const SecurityPolicy& policy);
    void Shutdown();
    
    // 执行安全策略
    bool ExecuteSafe(const std::string& code, 
                     std::chrono::milliseconds timeout = std::chrono::seconds(5));
    
    // 当前执行状态
    bool IsExecuting() const;
    void ForceStop();
    
    // 资源使用监控
    size_t GetMemoryUsage() const;
    std::chrono::milliseconds GetExecutionTime() const;
    int GetInstructionCount() const;
    
private:
    lua_State* m_L = nullptr;
    SecurityPolicy m_policy;
    
    // 安全钩子
    static void MemoryHook(lua_State* L, lua_Debug* ar);
    static void TimeoutHook(lua_State* L, lua_Debug* ar);
    static void InstructionHook(lua_State* L, lua_Debug* ar);
    
    // 替换危险函数
    void ReplaceDangerousFunctions();
    void SetupRestrictedLibraries();
    void SetupFileSandbox();
    void SetupNetworkSandbox();
};

// 安全IO封装
class SecureFileIO {
public:
    SecureFileIO(const std::vector<std::string>& allowedPaths,
                 const std::vector<std::string>& blockedPaths);
    
    // 路径验证
    bool IsPathAllowed(const std::string& path, bool forWrite) const;
    std::string NormalizePath(const std::string& path) const;
    
    // 文件操作
    FILE* Open(const std::string& path, const char* mode);
    bool Exists(const std::string& path) const;
    bool Remove(const std::string& path);
    bool Rename(const std::string& oldPath, const std::string& newPath);
    std::vector<std::string> ListDirectory(const std::string& path);
    
private:
    std::vector<std::string> m_allowedReadPaths;
    std::vector<std::string> m_allowedWritePaths;
    std::vector<std::string> m_blockedPaths;
};

// 网络安全
class SecureHttpClient {
public:
    struct SecurityConfig {
        bool httpsOnly = true;
        bool verifySSL = true;
        std::vector<std::string> allowedDomains;
        std::vector<std::string> blockedDomains;
        std::vector<std::string> certificatePins;
        int maxRequestsPerSecond = 10;
        std::chrono::seconds requestTimeout = std::chrono::seconds(30);
    };
    
    void SetConfig(const SecurityConfig& config);
    
    // 安全的HTTP请求
    HttpResponse Request(const HttpRequest& request);
    
    // 域名验证
    bool IsDomainAllowed(const std::string& domain) const;
    
    // 证书验证回调
    bool VerifyCertificate(const std::string& host, const std::vector<uint8_t>& cert);
    
private:
    SecurityConfig m_config;
    RateLimiter m_rateLimiter;
    std::unordered_map<std::string, std::vector<uint8_t>> m_pinnedCerts;
};

}} // namespace LuaUI::Security
```

### 1.3 Lua安全实现细节

```cpp
void LuaSandbox::ReplaceDangerousFunctions() {
    // 移除危险的全局函数
    lua_pushnil(m_L); lua_setglobal(m_L, "dofile");
    lua_pushnil(m_L); lua_setglobal(m_L, "loadfile");
    lua_pushnil(m_L); lua_setglobal(m_L, "load");
    lua_pushnil(m_L); lua_setglobal(m_L, "loadstring");
    
    // 限制require路径
    lua_getglobal(m_L, "package");
    if (lua_istable(m_L, -1)) {
        // 限制搜索路径
        lua_pushstring(m_L, "./?.lua;./?/init.lua");
        lua_setfield(m_L, -2, "path");
        
        // 禁用C库加载
        lua_pushstring(m_L, "");
        lua_setfield(m_L, -2, "cpath");
        
        // 替换加载器
        lua_getfield(m_L, -1, "searchers");
        if (lua_istable(m_L, -1)) {
            // 只保留Lua加载器，移除C加载器
            int len = lua_rawlen(m_L, -1);
            for (int i = len; i >= 3; i--) {
                lua_pushnil(m_L);
                lua_rawseti(m_L, -2, i);
            }
        }
        lua_pop(m_L, 1);
    }
    lua_pop(m_L, 1);
}

void LuaSandbox::SetupRestrictedLibraries() {
    // 限制IO库
    lua_getglobal(m_L, "io");
    if (lua_istable(m_L, -1)) {
        // 只保留安全的函数
        lua_pushnil(m_L);
        while (lua_next(m_L, -2)) {
            const char* name = lua_tostring(m_L, -2);
            bool keep = false;
            static const char* allowed[] = {"lines", "read", "tmpfile", "type", "write"};
            for (auto* a : allowed) {
                if (strcmp(name, a) == 0) { keep = true; break; }
            }
            if (!keep) {
                lua_pushnil(m_L);
                lua_setfield(m_L, -4, name);
            }
            lua_pop(m_L, 1);
        }
        
        // 替换open为安全版本
        lua_pushcfunction(m_L, SecureIOOpen);
        lua_setfield(m_L, -2, "open");
    }
    lua_pop(m_L, 1);
    
    // 限制OS库
    lua_getglobal(m_L, "os");
    if (lua_istable(m_L, -1)) {
        // 只保留时间相关函数
        lua_pushnil(m_L);
        while (lua_next(m_L, -2)) {
            const char* name = lua_tostring(m_L, -2);
            bool keep = false;
            static const char* allowed[] = {"clock", "date", "difftime", "time"};
            for (auto* a : allowed) {
                if (strcmp(name, a) == 0) { keep = true; break; }
            }
            if (!keep) {
                lua_pushnil(m_L);
                lua_setfield(m_L, -4, name);
            }
            lua_pop(m_L, 1);
        }
    }
    lua_pop(m_L, 1);
    
    // 移除debug库（或限制）
    if (!m_policy.allowDebug) {
        lua_pushnil(m_L);
        lua_setglobal(m_L, "debug");
    }
}

// 内存监控
void LuaSandbox::MemoryHook(lua_State* L, lua_Debug* ar) {
    LuaSandbox* self = static_cast<LuaSandbox*>(lua_touserdata(L, lua_upvalueindex(1)));
    
    size_t usedKB = lua_gc(L, LUA_GCCOUNT, 0);
    if (usedKB > self->m_policy.maxMemoryMB * 1024) {
        luaL_error(L, "Memory limit exceeded: %zu KB used", usedKB);
    }
}

// 执行超时监控
void LuaSandbox::TimeoutHook(lua_State* L, lua_Debug* ar) {
    LuaSandbox* self = static_cast<LuaSandbox*>(lua_touserdata(L, lua_upvalueindex(1)));
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - self->m_startTime);
    
    if (elapsed > self->m_policy.maxExecutionTime) {
        luaL_error(L, "Execution timeout after %lld ms", elapsed.count());
    }
}
```

## 2. 性能优化

### 2.1 渲染优化

```cpp
namespace LuaUI {
namespace Performance {

class RenderOptimizer {
public:
    // 脏矩形渲染
    class DirtyRegionManager {
    public:
        void InvalidateRect(const Rect& rect);
        void InvalidateAll();
        bool IsDirty(const Rect& rect) const;
        const std::vector<Rect>& GetDirtyRects() const;
        void Clear();
        void Optimize();  // 合并重叠矩形
        
    private:
        std::vector<Rect> m_dirtyRects;
    };
    
    // 绘制调用批处理
    class DrawCallBatcher {
    public:
        void BeginBatch();
        void AddDrawCall(const DrawCall& call);
        void EndBatch(IRenderTarget* target);
        
    private:
        std::vector<DrawCall> m_pendingCalls;
        bool m_batching = false;
    };
    
    // 缓存管理
    class CacheManager {
    public:
        // 元素缓存
        void CacheElement(UIElement* element);
        void InvalidateCache(UIElement* element);
        void InvalidateAllCache();
        bool IsCached(UIElement* element) const;
        void RenderFromCache(UIElement* element, IRenderTarget* target);
        
        // 缓存策略
        void SetMaxCacheSize(size_t bytes);
        void SetCacheTTL(std::chrono::seconds ttl);
        void CleanupExpiredCache();
        
    private:
        struct CacheEntry {
            ComPtr<ID2D1Bitmap> bitmap;
            std::chrono::steady_clock::time_point created;
            std::chrono::steady_clock::time_point lastUsed;
            Size size;
        };
        
        std::unordered_map<UIElement*, CacheEntry> m_cache;
        size_t m_maxCacheSize = 256 * 1024 * 1024;  // 256MB
        std::chrono::seconds m_cacheTTL = std::chrono::seconds(30);
    };
    
    // 纹理图集
    class TextureAtlas {
    public:
        void AddImage(Image* image);
        void Pack();
        Rect GetUVRect(Image* image) const;
        ID2D1Bitmap* GetAtlasTexture() const;
        
    private:
        struct AtlasNode {
            Rect rect;
            bool used = false;
            std::unique_ptr<AtlasNode> left;
            std::unique_ptr<AtlasNode> right;
        };
        
        std::unique_ptr<AtlasNode> m_root;
        ComPtr<ID2D1Bitmap> m_texture;
        std::unordered_map<Image*, Rect> m_uvRects;
    };
    
    // 自适应质量
    class AdaptiveQuality {
    public:
        enum class QualityLevel {
            Low,      // 30fps, 低精度文本, 无抗锯齿
            Medium,   // 60fps, 标准质量
            High      // 60fps+, 高质量
        };
        
        void Update(float frameTime);
        QualityLevel GetCurrentQuality() const { return m_currentQuality; }
        
        // 质量设置
        void SetTargetFrameRate(int fps);
        void SetQualityRange(QualityLevel min, QualityLevel max);
        
    private:
        QualityLevel m_currentQuality = QualityLevel::High;
        QualityLevel m_minQuality = QualityLevel::Low;
        QualityLevel m_maxQuality = QualityLevel::High;
        
        int m_targetFrameRate = 60;
        float m_frameTimeThreshold = 1000.0f / 55.0f;  // 55fps threshold
        
        // 滑动平均
        std::deque<float> m_frameTimeHistory;
        static constexpr size_t HISTORY_SIZE = 60;
    };
    
    // 字距缓存
    class GlyphCache {
    public:
        struct GlyphKey {
            FontFamily font;
            float size;
            uint32_t codepoint;
            FontWeight weight;
            FontStyle style;
            
            bool operator==(const GlyphKey& other) const;
            struct Hash {
                size_t operator()(const GlyphKey& key) const;
            };
        };
        
        struct GlyphInfo {
            ComPtr<ID2D1Bitmap> bitmap;
            Point offset;
            float advance;
        };
        
        const GlyphInfo* GetGlyph(const GlyphKey& key);
        void PreloadGlyphs(const std::wstring& text, const TextFormat& format);
        void Clear();
        
    private:
        std::unordered_map<GlyphKey, GlyphInfo, GlyphKey::Hash> m_cache;
        size_t m_maxCacheSize = 64 * 1024 * 1024;  // 64MB
    };
};

}} // namespace LuaUI::Performance
```

### 2.2 内存管理

```cpp
namespace LuaUI {
namespace Performance {

class MemoryManager {
public:
    // 对象池
    template<typename T>
    class ObjectPool {
    public:
        using Factory = std::function<T*()>;
        using Resetter = std::function<void(T*)>;
        
        ObjectPool(size_t initialSize, Factory factory, Resetter resetter);
        ~ObjectPool();
        
        T* Acquire();
        void Release(T* obj);
        void Clear();
        size_t GetPoolSize() const;
        
    private:
        std::vector<T*> m_available;
        std::unordered_set<T*> m_inUse;
        Factory m_factory;
        Resetter m_resetter;
        std::mutex m_mutex;
    };
    
    // 内存池
    class LinearAllocator {
    public:
        explicit LinearAllocator(size_t blockSize);
        ~LinearAllocator();
        
        void* Allocate(size_t size, size_t alignment = 8);
        void Reset();
        size_t GetUsedSize() const;
        size_t GetTotalSize() const;
        
    private:
        struct Block {
            std::unique_ptr<uint8_t[]> memory;
            size_t size;
            size_t used;
        };
        
        std::vector<Block> m_blocks;
        size_t m_defaultBlockSize;
    };
    
    // 智能内存管理
    class MemoryTracker {
    public:
        static MemoryTracker& GetInstance();
        
        void TrackAllocation(void* ptr, size_t size, const char* type, const char* file, int line);
        void TrackDeallocation(void* ptr);
        
        void DumpMemoryReport();
        void CheckLeaks();
        
        struct AllocationInfo {
            size_t size;
            const char* type;
            const char* file;
            int line;
            std::chrono::steady_clock::time_point time;
        };
        
    private:
        std::unordered_map<void*, AllocationInfo> m_allocations;
        std::mutex m_mutex;
    };
    
    // 纹理压缩
    class TextureCompressor {
    public:
        enum class Format {
            BC1,  // DXT1 - 不透明或1bit alpha
            BC2,  // DXT3 - 显式alpha
            BC3,  // DXT5 - 插值alpha
            BC7,  // 高质量压缩
        };
        
        static std::vector<uint8_t> Compress(const Image& image, Format format);
        static std::unique_ptr<Image> Decompress(const void* data, size_t size, 
                                                  int width, int height, Format format);
    };
};

}} // namespace LuaUI::Performance
```

### 2.3 多线程优化

```cpp
namespace LuaUI {
namespace Performance {

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads);
    ~ThreadPool();
    
    template<typename Func, typename... Args>
    auto Submit(Func&& func, Args&&... args) -> std::future<decltype(func(args...))> {
        using ReturnType = decltype(func(args...));
        
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
        );
        
        std::future<ReturnType> result = task->get_future();
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_tasks.emplace([task]() { (*task)(); });
        }
        m_condition.notify_one();
        
        return result;
    }
    
    void WaitForAll();
    size_t GetPendingCount() const;
    
private:
    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;
    mutable std::mutex m_mutex;
    std::condition_variable m_condition;
    bool m_stop = false;
};

// 并行算法
class Parallel {
public:
    // 并行for
    template<typename Index, typename Func>
    static void For(Index start, Index end, Func&& func) {
        size_t numThreads = std::thread::hardware_concurrency();
        Index range = end - start;
        Index chunkSize = (range + numThreads - 1) / numThreads;
        
        std::vector<std::future<void>> futures;
        for (size_t i = 0; i < numThreads; i++) {
            Index chunkStart = start + i * chunkSize;
            Index chunkEnd = std::min(chunkStart + chunkSize, end);
            
            if (chunkStart < chunkEnd) {
                futures.push_back(ThreadPool::GetInstance().Submit([func, chunkStart, chunkEnd]() {
                    for (Index j = chunkStart; j < chunkEnd; j++) {
                        func(j);
                    }
                }));
            }
        }
        
        for (auto& f : futures) {
            f.wait();
        }
    }
    
    // 并行reduce
    template<typename Iterator, typename T, typename Func>
    static T Reduce(Iterator begin, Iterator end, T init, Func&& func) {
        // 分段reduce后合并
        // ...
    }
};

}} // namespace LuaUI::Performance
```

### 2.4 启动优化

```cpp
namespace LuaUI {
namespace Performance {

class StartupOptimizer {
public:
    // 延迟加载
    class LazyLoader {
    public:
        template<typename T>
        class Lazy {
        public:
            using Factory = std::function<std::unique_ptr<T>()>;
            
            explicit Lazy(Factory factory) : m_factory(factory) {}
            
            T* get() {
                if (!m_instance) {
                    std::call_once(m_once, [this]() {
                        m_instance = m_factory();
                    });
                }
                return m_instance.get();
            }
            
            T* operator->() { return get(); }
            T& operator*() { return *get(); }
            
        private:
            Factory m_factory;
            std::unique_ptr<T> m_instance;
            std::once_flag m_once;
        };
    };
    
    // 异步初始化
    class AsyncInitializer {
    public:
        void Register(std::function<void()> init, int priority = 0);
        void Run();
        void RunAsync();
        void WaitForCompletion();
        
    private:
        struct InitTask {
            std::function<void()> func;
            int priority;
        };
        
        std::vector<InitTask> m_tasks;
    };
    
    // 预加载资源
    class ResourcePreloader {
    public:
        void PreloadImage(const std::string& path);
        void PreloadFont(const std::string& family);
        void PreloadTheme(const std::string& themeName);
        
        void StartPreloading();
        void WaitForPreload();
        
    private:
        std::vector<std::future<void>> m_preloadFutures;
    };
    
    // 启动时间分析
    class StartupProfiler {
    public:
        static StartupProfiler& GetInstance();
        
        void BeginPhase(const std::string& name);
        void EndPhase(const std::string& name);
        
        void DumpReport();
        
    private:
        struct Phase {
            std::string name;
            std::chrono::steady_clock::time_point start;
            std::chrono::milliseconds duration;
        };
        
        std::vector<Phase> m_phases;
        std::stack<std::string> m_activePhases;
    };
};

}} // namespace LuaUI::Performance
```

## 3. 性能监控

```cpp
namespace LuaUI {
namespace Performance {

class PerformanceMonitor {
public:
    static PerformanceMonitor& GetInstance();
    
    // FPS监控
    void UpdateFrame();
    float GetCurrentFPS() const;
    float GetAverageFPS() const;
    float GetMinFPS() const;
    float GetMaxFPS() const;
    
    // 内存监控
    struct MemoryStats {
        size_t workingSet;
        size_t privateBytes;
        size_t managedHeap;
        size_t textureMemory;
        size_t cacheMemory;
    };
    MemoryStats GetMemoryStats() const;
    
    // CPU监控
    struct CPUStats {
        float totalUsage;
        float appUsage;
        int threadCount;
    };
    CPUStats GetCPUStats() const;
    
    // GPU监控
    struct GPUStats {
        float gpuUsage;
        size_t dedicatedMemoryUsed;
        size_t sharedMemoryUsed;
        int drawCallCount;
        int triangleCount;
    };
    GPUStats GetGPUStats() const;
    
    // 渲染统计
    struct RenderStats {
        int drawCalls;
        int triangles;
        int textureSwitches;
        int shaderSwitches;
        int renderTargetSwitches;
        std::chrono::milliseconds cpuTime;
        std::chrono::milliseconds gpuTime;
    };
    RenderStats GetRenderStats() const;
    void ResetRenderStats();
    
    // 性能分析器
    class Profiler {
    public:
        explicit Profiler(const char* name);
        ~Profiler();
        
        void Mark(const char* label);
        
    private:
        const char* m_name;
        std::chrono::steady_clock::time_point m_start;
        std::vector<std::pair<const char*, std::chrono::steady_clock::time_point>> m_marks;
    };
    
    // 显示调试信息
    void ShowDebugOverlay(bool show);
    void RenderDebugOverlay(IRenderTarget* target);
    
private:
    PerformanceMonitor();
    
    // FPS计算
    std::deque<std::chrono::steady_clock::time_point> m_frameTimes;
    static constexpr size_t FPS_HISTORY_SIZE = 120;
    
    // 统计数据
    mutable std::mutex m_statsMutex;
    RenderStats m_renderStats;
};

}} // namespace LuaUI::Performance
```

## 4. 性能优化建议

### 4.1 渲染优化清单

- [ ] 使用脏矩形渲染减少重绘
- [ ] 对复杂控件启用离屏缓存
- [ ] 合并相同材质的绘制调用
- [ ] 使用纹理图集减少切换
- [ ] 启用自适应质量
- [ ] 对不可见元素跳过渲染
- [ ] 使用GPU加速效果

### 4.2 内存优化清单

- [ ] 使用对象池减少分配
- [ ] 及时释放未使用资源
- [ ] 使用纹理压缩
- [ ] 限制缓存大小
- [ ] 监控内存泄漏
- [ ] 使用智能指针管理生命周期

### 4.3 启动优化清单

- [ ] 延迟加载非必要资源
- [ ] 异步初始化模块
- [ ] 预加载关键资源
- [ ] 压缩资源文件
- [ ] 使用增量更新
- [ ] 减少依赖库大小

### 4.4 Lua优化建议

```lua
-- 1. 使用局部变量访问全局变量
local UI = UI  -- 缓存全局
local getControl = UI.getControl  -- 缓存方法

-- 2. 避免在循环中创建闭包
local function processItem(item)
    -- 处理逻辑
end

for _, item in ipairs(items) do
    processItem(item)  -- 好
    -- 不要: (function() processItem(item) end)()
end

-- 3. 使用table预分配
local list = {}
for i = 1, 1000 do
    list[i] = value  -- 比 table.insert 快
end

-- 4. 缓存频繁访问的属性
local width = element.width
for i = 1, n do
    -- 使用 width 而不是每次都访问 element.width
end

-- 5. 批量更新
ViewModel:beginUpdate()
for _, item in ipairs(items) do
    ViewModel:addItem(item)
end
ViewModel:endUpdate()  -- 只触发一次更新

-- 6. 延迟加载数据
function ViewModel:loadData()
    Task.run(function()
        -- 后台加载
        local data = fetchData()
        return data
    end):continueOnUI(function(data)
        -- UI线程更新
        self:setData(data)
    end)
end
```
