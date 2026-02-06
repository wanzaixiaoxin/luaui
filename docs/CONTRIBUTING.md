# 贡献指南

感谢您对LuaUI项目的关注！

## 如何贡献

### 报告问题

如果您发现了bug或有功能建议，请在GitHub上创建Issue。

### 提交代码

1. Fork本仓库
2. 创建您的特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交您的更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建Pull Request

## 代码规范

### C++代码

- 遵循C++11标准
- 使用Doxygen风格的注释
- 使用4空格缩进
- 类名使用大驼峰命名法 (PascalCase)
- 函数名使用大驼峰命名法 (PascalCase)
- 变量名使用小驼峰命名法 (camelCase)
- 成员变量使用m_前缀

### 代码示例

```cpp
/**
 * @brief 控件类
 * @details 实现基础控件功能
 */
class Control {
public:
    /**
     * @brief 构造函数
     */
    Control();
    
    /**
     * @brief 获取控件ID
     * @return 控件ID
     */
    std::string getId() const;

private:
    std::string m_id;  ///< 控件ID
};
```

## 开发环境

- Visual Studio 2019/2022
- CMake 3.15+
- Lua 5.4.4
- TinyXML2 9.0.0

## 许可证

提交代码即表示您同意您的代码将根据Apache License 2.0授权。
