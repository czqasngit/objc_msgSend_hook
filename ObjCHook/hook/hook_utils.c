//
//  hook_utils.c
//  ObjCHook
//
//  Created by legendry on 2019/4/1.
//  Copyright © 2019 legendry. All rights reserved.
//

#include "hook_utils.h"
#include <objc/runtime.h>
#include <sys/time.h>
#include <string.h>

const char *hook_getObjectClassName(void *_target) {
    return class_getName(object_getClass(_target));
}
const char *hook_getMethodName(Class cls, SEL _cmd) {
    Method m = class_getClassMethod(cls, _cmd);
    struct objc_method_description *desc = method_getDescription(m);
    return "";
}
/// 获取当前系统时间
uint64_t hook_getMillisecond() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_usec / 1000 + tv.tv_sec * 1000;
}

bool hook_has_prefix(char *target, char *prefix) {
    uint64_t count = strlen(prefix);
    bool ret = true;
    if(strlen(target) < count)
        return false;
    for (int i = 0; i < count; i ++) {
        if (target[i] != prefix[i]) {
            ret = false;
            break;
        }
    }
    return ret;
}
