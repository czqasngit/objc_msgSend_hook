//
//  hook_.c
//  ObjCHook
//
//  Created by legendry on 2019/3/28.
//  Copyright © 2019 legendry. All rights reserved.
//

#include "hook.h"
#if __aarch64__
#include "hook_core_arm64.h"
#else
#include "hook_core_x64.h"
#endif

/// 开始hook
void start_objc_msgSend_hook(void) {
#if __aarch64__
    arm64_start_objc_msgSend_hook();
#else
    x64_start_objc_msgSend_hook();
#endif
}
/// 停止
void stop_objc_msgSend_hook(void) {
#if __aarch64__
    arm64_stop_objc_msgSend_hook();
#else
    x64_stop_objc_msgSend_hook();
#endif
}
