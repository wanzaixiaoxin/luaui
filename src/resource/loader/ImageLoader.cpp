/**
 * @file ImageLoader.cpp
 * @brief 图片加载器实现
 */

#include "resource/loader/ImageLoader.h"
#include "core/Utils.h"

// Windows headers for memory management
#include <windows.h>
#include <ole2.h>

namespace LuaUI {
namespace Resource {
namespace Loader {

ImageLoader::ImageLoader() {
}

ImageLoader::~ImageLoader() {
    clearCache();
}

ImageInfo* ImageLoader::loadImage(const std::string& filepath) {
    return loadImageWithCache(filepath);
}

ImageInfo* ImageLoader::loadImageFromMemory(const void* data, size_t size) {
    if (!data || size == 0) {
        setLastError("Invalid data or size");
        return nullptr;
    }
    
    ImageInfo* info = new ImageInfo();
    
    // 创建MFC图像对象
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, size);
    void* pData = GlobalLock(hGlobal);
    memcpy(pData, data, size);
    GlobalUnlock(hGlobal);
    
    IStream* pStream = nullptr;
    if (CreateStreamOnHGlobal(hGlobal, TRUE, &pStream) == S_OK) {
        info->image = new CImage();
        if (info->image->Load(pStream) == S_OK) {
            info->width = info->image->GetWidth();
            info->height = info->image->GetHeight();
            info->bitCount = info->image->GetBPP();
            info->format = getImageFormat("");
        }
        pStream->Release();
    }
    
    GlobalFree(hGlobal);
    
    if (!info->image || info->width == 0) {
        delete info;
        setLastError("Failed to load image from memory");
        return nullptr;
    }
    
    return info;
}

ImageInfo* ImageLoader::loadImageWithCache(const std::string& filepath, 
                                           const std::string& cacheKey) {
    // 确定缓存键
    std::string key = cacheKey.empty() ? filepath : cacheKey;
    
    // 检查缓存
    std::map<std::string, ImageInfo*>::iterator it = m_imageCache.find(key);
    if (it != m_imageCache.end()) {
        return it->second;
    }
    
    // 加载图片
    ImageFormat format = getImageFormat(filepath);
    ImageInfo* info = nullptr;
    
    switch (format) {
        case FORMAT_BMP:
            info = loadBMP(filepath);
            break;
        case FORMAT_PNG:
            info = loadPNG(filepath);
            break;
        case FORMAT_JPG:
            info = loadJPEG(filepath);
            break;
        default:
            // 尝试使用MFC加载
            info = new ImageInfo();
            info->image = new CImage();
            
            if (SUCCEEDED(info->image->Load(Utils::EncodingUtils::utf8ToUtf16(filepath).c_str()))) {
                info->path = filepath;
                info->width = info->image->GetWidth();
                info->height = info->image->GetHeight();
                info->bitCount = info->image->GetBPP();
                info->format = format;
            } else {
                delete info;
                info = nullptr;
                setLastError("Failed to load image: " + filepath);
            }
            break;
    }
    
    // 添加到缓存
    if (info) {
        m_imageCache[key] = info;
    }
    
    return info;
}

bool ImageLoader::unloadImage(const std::string& filepath) {
    return unloadImageByKey(filepath);
}

bool ImageLoader::unloadImageByKey(const std::string& cacheKey) {
    std::map<std::string, ImageInfo*>::iterator it = m_imageCache.find(cacheKey);
    if (it != m_imageCache.end()) {
        if (it->second) {
            if (it->second->image) {
                delete it->second->image;
            }
            delete it->second;
        }
        m_imageCache.erase(it);
        return true;
    }
    return false;
}

void ImageLoader::clearCache() {
    for (std::map<std::string, ImageInfo*>::iterator it = m_imageCache.begin();
         it != m_imageCache.end(); ++it) {
        if (it->second) {
            if (it->second->image) {
                delete it->second->image;
            }
            delete it->second;
        }
    }
    m_imageCache.clear();
}

ImageInfo* ImageLoader::getImageInfo(const std::string& filepath) {
    return getImageInfoByKey(filepath);
}

ImageInfo* ImageLoader::getImageInfoByKey(const std::string& cacheKey) {
    std::map<std::string, ImageInfo*>::iterator it = m_imageCache.find(cacheKey);
    if (it != m_imageCache.end()) {
        return it->second;
    }
    return nullptr;
}

bool ImageLoader::isLoaded(const std::string& filepath) const {
    return m_imageCache.find(filepath) != m_imageCache.end();
}

ImageFormat ImageLoader::getImageFormat(const std::string& filepath) {
    std::string ext = Utils::FileUtils::getFileExtension(filepath);
    ext = Utils::StringUtils::toLower(ext);
    
    if (ext == "bmp") {
        return FORMAT_BMP;
    } else if (ext == "png") {
        return FORMAT_PNG;
    } else if (ext == "jpg" || ext == "jpeg") {
        return FORMAT_JPG;
    } else if (ext == "gif") {
        return FORMAT_GIF;
    } else if (ext == "ico") {
        return FORMAT_ICO;
    }
    
    return FORMAT_UNKNOWN;
}

std::string ImageLoader::getFormatExtension(ImageFormat format) {
    switch (format) {
        case FORMAT_BMP:  return ".bmp";
        case FORMAT_PNG:  return ".png";
        case FORMAT_JPG:  return ".jpg";
        case FORMAT_GIF:  return ".gif";
        case FORMAT_ICO:  return ".ico";
        default:          return "";
    }
}

std::string ImageLoader::getLastError() const {
    return m_lastError;
}

ImageInfo* ImageLoader::loadBMP(const std::string& filepath) {
    ImageInfo* info = new ImageInfo();
    info->image = new CImage();
    
    std::wstring wpath = Utils::EncodingUtils::utf8ToUtf16(filepath);
    
    if (SUCCEEDED(info->image->Load(wpath.c_str()))) {
        info->path = filepath;
        info->width = info->image->GetWidth();
        info->height = info->image->GetHeight();
        info->bitCount = info->image->GetBPP();
        info->format = FORMAT_BMP;
        return info;
    }
    
    delete info;
    setLastError("Failed to load BMP image");
    return nullptr;
}

ImageInfo* ImageLoader::loadPNG(const std::string& filepath) {
    ImageInfo* info = new ImageInfo();
    info->image = new CImage();
    
    std::wstring wpath = Utils::EncodingUtils::utf8ToUtf16(filepath);
    
    if (SUCCEEDED(info->image->Load(wpath.c_str()))) {
        info->path = filepath;
        info->width = info->image->GetWidth();
        info->height = info->image->GetHeight();
        info->bitCount = info->image->GetBPP();
        info->format = FORMAT_PNG;
        return info;
    }
    
    delete info;
    setLastError("Failed to load PNG image");
    return nullptr;
}

ImageInfo* ImageLoader::loadJPEG(const std::string& filepath) {
    ImageInfo* info = new ImageInfo();
    info->image = new CImage();
    
    std::wstring wpath = Utils::EncodingUtils::utf8ToUtf16(filepath);
    
    if (SUCCEEDED(info->image->Load(wpath.c_str()))) {
        info->path = filepath;
        info->width = info->image->GetWidth();
        info->height = info->image->GetHeight();
        info->bitCount = info->image->GetBPP();
        info->format = FORMAT_JPG;
        return info;
    }
    
    delete info;
    setLastError("Failed to load JPEG image");
    return nullptr;
}

void ImageLoader::setLastError(const std::string& error) {
    m_lastError = error;
}

} // namespace Loader
} // namespace Resource
} // namespace LuaUI
