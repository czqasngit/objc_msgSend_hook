//
//  hook_core.h
//  ObjCHook
//
//  Created by legendry on 2019/4/1.
//  Copyright © 2019 legendry. All rights reserved.
//

#ifndef hook_core_h
#define hook_core_h
#include <stdio.h>
#include <objc/runtime.h>
///hook_core 实现了hook的方法统计,具体的hook在不同的CPU下面完成
///intel x64在llvm 2.0之后不再支持嵌入式asm代码
/// 后
uintptr_t after_objc_msgSend(void);
/// 前
void before_objc_msgSend(id SELF, SEL _cmd, uintptr_t lr);
/// 开始hook
void start_hook(void *hook_objc_msgSend, void **origin_objc_msgSend);
#endif /* hook_core_h */
