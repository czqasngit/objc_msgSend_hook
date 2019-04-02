//
//  hook_core_arm64.c
//  ObjCHook
//
//  Created by legendry on 2019/4/1.
//  Copyright © 2019 legendry. All rights reserved.
//

#include "hook_core_arm64.h"
#import "fishhook.h"
#import <objc/runtime.h>
#import <objc/message.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dispatch/dispatch.h>
#include "hook_utils.h"
#include "hook_core.h"

#if __aarch64__
static id (*orig_objc_msgSend)(void);
// 将参数value(地址)传递给x12寄存器
// blr开始执行
#define call(b, value) \
__asm volatile ("mov x12, %0\n" :: "r"(value)); \
__asm volatile (#b " x12\n");

/// 依次将寄存器数据入栈
#define save() \
__asm volatile ( \
"stp x8, x9, [sp, #-16]!\n" \
"stp x6, x7, [sp, #-16]!\n" \
"stp x4, x5, [sp, #-16]!\n" \
"stp x2, x3, [sp, #-16]!\n" \
"stp x0, x1, [sp, #-16]!\n" \
);

/// 依次将寄存器数据出栈
#define load() \
__asm volatile ( \
"ldp x0, x1, [sp], #16\n" \
"ldp x2, x3, [sp], #16\n" \
"ldp x4, x5, [sp], #16\n" \
"ldp x6, x7, [sp], #16\n" \
"ldp x8, x9, [sp], #16\n" );

/// 程序执行完成,返回将继续执行lr中的函数
#define ret() __asm volatile ("ret\n");

static void hook_objc_msgSend() {
    /// before之前保存objc_msgSend的参数
    save()
    /// 将objc_msgSend执行的下一个函数地址传递给before_objc_msgSend的第二个参数x0 self, x1 _cmd, x2: lr address
    __asm volatile ("mov x2, lr\n");
    /// 执行before_objc_msgSend
    call(blr, &before_objc_msgSend)
    /// 恢复objc_msgSend参数，并执行
    load()
    call(blr, orig_objc_msgSend)
    /// after之前保存objc_msgSend执行完成的参数
    save()
    /// 调用 after_objc_msgSend
    call(blr, &after_objc_msgSend)
    /// 将after_objc_msgSend返回的参数放入lr,恢复调用before_objc_msgSend前的lr地址
    __asm volatile ("mov lr, x0\n");
    /// 恢复objc_msgSend执行完成的参数
    load()
    /// 方法结束,继续执行lr
    ret()
}

///开始
void arm64_start_objc_msgSend_hook() {
    start_hook(hook_objc_msgSend, (void *)(&orig_objc_msgSend));
}
void arm64_stop_objc_msgSend_hook() {
    
}
#endif
