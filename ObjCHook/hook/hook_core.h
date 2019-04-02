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


uintptr_t after_objc_msgSend(void);
void before_objc_msgSend(id SELF, SEL _cmd, uintptr_t lr);
void start_hook(void *hook_objc_msgSend, void **origin_objc_msgSend);
#endif /* hook_core_h */
