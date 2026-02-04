@echo off
setlocal enabledelayedexpansion

:: 配置路径
set LUA_VERSION=5.4.6
set SRC_DIR=src
set OUT_DIR=build

:: 查找 Visual Studio 安装路径
set VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist %VSWHERE% (
    echo 错误：未找到 Visual Studio
    exit /b 1
)

for /f "usebackq tokens=*" %%i in (`%VSWHERE% -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set VS_PATH=%%i
)

:: 调用 vcvarsall.bat 设置环境变量
call "%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat" x64

:: 创建输出目录
if not exist %OUT_DIR% mkdir %OUT_DIR%

:: 编译选项说明：
:: -DLUA_BUILD_AS_DLL : 关键宏，启用 DLL 导出
:: -DLUA_LIB          : 标记为库编译
:: -wd4996            : 禁用 strdup 等警告

set CFLAGS=-O2 -MD -DLUA_BUILD_AS_DLL -DLUA_LIB -wd4996 -W3 -nologo
set LFLAGS=-DLL -incremental:no

:: 编译所有 C 文件为对象文件
echo 正在编译对象文件...
for %%f in (%SRC_DIR%\*.c) do (
    if not "%%~nf"=="luac" (
        if not "%%~nf"=="lua" (
            cl %CFLAGS% -c %%f -Fo%OUT_DIR%\%%~nf.obj
        )
    )
)

:: 链接为 lua54.dll（排除 lua.c 和 luac.c，这两个是解释器/编译器入口）
echo 正在链接 DLL...
link %LFLAGS% -out:%OUT_DIR%\lua54.dll %OUT_DIR%\*.obj -implib:%OUT_DIR%\lua54.lib

:: 编译解释器（可选）
cl %CFLAGS% %SRC_DIR%\lua.c -link %OUT_DIR%\lua54.lib -out:%OUT_DIR%\lua.exe

:: 编译编译器（可选）
cl %CFLAGS% %SRC_DIR%\luac.c -link %OUT_DIR%\lua54.lib -out:%OUT_DIR%\luac.exe

:: 复制头文件
copy %SRC_DIR%\lua.h %OUT_DIR%\
copy %SRC_DIR%\luaconf.h %OUT_DIR%\
copy %SRC_DIR%\lualib.h %OUT_DIR%\
copy %SRC_DIR%\lauxlib.h %OUT_DIR%\

echo.
echo 编译完成！输出文件在 %OUT_DIR% 目录：
dir %OUT_DIR%\*.dll %OUT_DIR%\*.lib %OUT_DIR%\*.exe

endlocal
pause