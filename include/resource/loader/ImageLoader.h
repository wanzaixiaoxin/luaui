/**
 * @file ImageLoader.h
 * @brief 图片加载器
 * @details 加载和管理图片资源
 * @version 1.0.0
 */

#ifndef LUAUI_IMAGELOADER_H
#define LUAUI_IMAGELOADER_H

#include <string>
#include <map>

// MFC forward declarations
class CImage;

namespace LuaUI {
namespace Resource {
namespace Loader {

/**
 * @brief 图片格式
 */
enum ImageFormat {
    FORMAT_BMP,   ///< BMP格式
    FORMAT_PNG,   ///< PNG格式
    FORMAT_JPG,   ///< JPEG格式
    FORMAT_GIF,   ///< GIF格式
    FORMAT_ICO,   ///< ICO格式
    FORMAT_UNKNOWN ///< 未知格式
};

/**
 * @brief 图片信息
 */
struct ImageInfo {
    std::string path;       ///< 图片路径
    int width;             ///< 宽度
    int height;            ///< 高度
    int bitCount;          ///< 位深度
    ImageFormat format;     ///< 格式
    CImage* image;         ///< MFC图像对象
    
    /**
     * @brief 构造函数
     */
    ImageInfo()
        : width(0)
        , height(0)
        , bitCount(0)
        , format(FORMAT_UNKNOWN)
        , image(nullptr)
    {
    }
};

/**
 * @brief 图片加载器类
 */
class ImageLoader {
public:
    /**
     * @brief 构造函数
     */
    ImageLoader();
    
    /**
     * @brief 析构函数
     */
    ~ImageLoader();
    
    /**
     * @brief 加载图片
     * @param filepath 图片文件路径
     * @return 图片信息指针，失败返回nullptr
     */
    ImageInfo* loadImage(const std::string& filepath);
    
    /**
     * @brief 从内存加载图片
     * @param data 图片数据
     * @param size 数据大小
     * @return 图片信息指针，失败返回nullptr
     */
    ImageInfo* loadImageFromMemory(const void* data, size_t size);
    
    /**
     * @brief 加载图片并缓存
     * @param filepath 图片文件路径
     * @param cacheKey 缓存键名（如果为空则使用文件路径）
     * @return 图片信息指针，失败返回nullptr
     */
    ImageInfo* loadImageWithCache(const std::string& filepath, const std::string& cacheKey = "");
    
    /**
     * @brief 卸载图片
     * @param filepath 图片文件路径
     * @return 成功返回true，失败返回false
     */
    bool unloadImage(const std::string& filepath);
    
    /**
     * @brief 卸载图片（通过缓存键）
     * @param cacheKey 缓存键名
     * @return 成功返回true，失败返回false
     */
    bool unloadImageByKey(const std::string& cacheKey);
    
    /**
     * @brief 清除所有图片缓存
     */
    void clearCache();
    
    /**
     * @brief 获取图片信息
     * @param filepath 图片文件路径
     * @return 图片信息指针，如果不存在返回nullptr
     */
    ImageInfo* getImageInfo(const std::string& filepath);
    
    /**
     * @brief 获取图片信息（通过缓存键）
     * @param cacheKey 缓存键名
     * @return 图片信息指针，如果不存在返回nullptr
     */
    ImageInfo* getImageInfoByKey(const std::string& cacheKey);
    
    /**
     * @brief 检查图片是否已加载
     * @param filepath 图片文件路径
     * @return 已加载返回true，否则返回false
     */
    bool isLoaded(const std::string& filepath) const;
    
    /**
     * @brief 获取图片格式
     * @param filepath 图片文件路径
     * @return 图片格式
     */
    static ImageFormat getImageFormat(const std::string& filepath);
    
    /**
     * @brief 获取格式扩展名
     * @param format 图片格式
     * @return 扩展名
     */
    static std::string getFormatExtension(ImageFormat format);
    
    /**
     * @brief 获取最后一次错误信息
     * @return 错误信息
     */
    std::string getLastError() const;

private:
    std::map<std::string, ImageInfo*> m_imageCache;  ///< 图片缓存
    std::string m_lastError;                        ///< 最后一次错误信息
    
    /**
     * @brief 加载BMP图片
     * @param filepath 文件路径
     * @return 图片信息指针，失败返回nullptr
     */
    ImageInfo* loadBMP(const std::string& filepath);
    
    /**
     * @brief 加载PNG图片
     * @param filepath 文件路径
     * @return 图片信息指针，失败返回nullptr
     */
    ImageInfo* loadPNG(const std::string& filepath);
    
    /**
     * @brief 加载JPEG图片
     * @param filepath 文件路径
     * @return 图片信息指针，失败返回nullptr
     */
    ImageInfo* loadJPEG(const std::string& filepath);
    
    /**
     * @brief 设置错误信息
     * @param error 错误信息
     */
    void setLastError(const std::string& error);
};

} // namespace Loader
} // namespace Resource
} // namespace LuaUI

#endif // LUAUI_IMAGELOADER_H
