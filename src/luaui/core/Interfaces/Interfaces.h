#pragma once

/**
 * @brief LuaUI 核心接口层
 * 
 * 按 SOLID 原则设计：
 * - SRP: 每个接口职责单一
 * - ISP: 接口分离，客户端不依赖不需要的方法
 * - DIP: 高层模块依赖这些抽象接口，而非具体实现
 */

#include "IControl.h"
#include "IRenderable.h"
#include "ILayoutable.h"
#include "IInputHandler.h"
#include "IStyleable.h"
#include "INativeWindow.h"
