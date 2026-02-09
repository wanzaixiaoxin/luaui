#pragma once

#include <unordered_map>
#include <memory>
#include <mutex>
#include <string>

namespace luaui {
namespace rendering {

// LRU cache for rendering resources
template<typename Key, typename Value>
class ResourceCache {
public:
    struct Entry {
        std::shared_ptr<Value> value;
        size_t size = 0;
        uint64_t lastAccess = 0;
        uint32_t accessCount = 0;
    };
    
    explicit ResourceCache(size_t maxSize = 64 * 1024 * 1024) : m_maxSize(maxSize) {}
    
    std::shared_ptr<Value> Get(const Key& key) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_entries.find(key);
        if (it != m_entries.end()) {
            it->second.lastAccess = ++m_accessCounter;
            it->second.accessCount++;
            return it->second.value;
        }
        return nullptr;
    }
    
    void Put(const Key& key, std::shared_ptr<Value> value, size_t size = 0) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Evict if necessary
        while (m_currentSize + size > m_maxSize && !m_entries.empty()) {
            EvictLRU();
        }
        
        Entry entry;
        entry.value = value;
        entry.size = size;
        entry.lastAccess = ++m_accessCounter;
        
        auto it = m_entries.find(key);
        if (it != m_entries.end()) {
            m_currentSize -= it->second.size;
        }
        
        m_entries[key] = std::move(entry);
        m_currentSize += size;
    }
    
    void Remove(const Key& key) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_entries.find(key);
        if (it != m_entries.end()) {
            m_currentSize -= it->second.size;
            m_entries.erase(it);
        }
    }
    
    void Clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_entries.clear();
        m_currentSize = 0;
    }
    
    void Trim() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Remove entries that haven't been accessed recently
        // (keep top 80% by access frequency)
        if (m_entries.size() < 10) return;
        
        // TODO: Implement more sophisticated trimming
    }
    
    size_t GetSize() const { return m_currentSize; }
    size_t GetMaxSize() const { return m_maxSize; }
    void SetMaxSize(size_t maxSize) { m_maxSize = maxSize; }
    
    size_t GetEntryCount() const { return m_entries.size(); }
    
private:
    void EvictLRU() {
        uint64_t oldestAccess = UINT64_MAX;
        typename std::unordered_map<Key, Entry>::iterator oldest = m_entries.end();
        
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
            if (it->second.lastAccess < oldestAccess) {
                oldestAccess = it->second.lastAccess;
                oldest = it;
            }
        }
        
        if (oldest != m_entries.end()) {
            m_currentSize -= oldest->second.size;
            m_entries.erase(oldest);
        }
    }
    
    std::unordered_map<Key, Entry> m_entries;
    size_t m_maxSize;
    size_t m_currentSize = 0;
    uint64_t m_accessCounter = 0;
    mutable std::mutex m_mutex;
};

// String hash for cache keys
struct StringHash {
    std::size_t operator()(const std::wstring& str) const {
        return std::hash<std::wstring>()(str);
    }
};

} // namespace rendering
} // namespace luaui
