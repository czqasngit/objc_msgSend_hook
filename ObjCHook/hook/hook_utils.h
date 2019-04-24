//
//  hook_utils.h
//  ObjCHook
//
//  Created by legendry on 2019/4/1.
//  Copyright © 2019 legendry. All rights reserved.
//

#ifndef hook_utils_h
#define hook_utils_h

#include <stdio.h>
#include <objc/runtime.h>

/// 返回对象的类名
/// _target: NSObject 对象
const char *hook_getObjectClassName(void *_target);
/// 获取类名
const char *hook_getMethodName(Class cls, SEL _cmd);
/// 获取当前系统时间
uint64_t hook_getMillisecond(void);
/// 判断target是否包含有prefix字符串
bool hook_has_prefix(char *target, char *prefix) ;
#endif /* hook_utils_h */
