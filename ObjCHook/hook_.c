//
//  hook_.c
//  ObjCHook
//
//  Created by legendry on 2019/3/28.
//  Copyright © 2019 legendry. All rights reserved.
//

#include "hook_.h"
#import "fishhook.h"
#import <objc/runtime.h>
#import <objc/message.h>
#import <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <objc/message.h>
#include <objc/runtime.h>
#include <dispatch/dispatch.h>
#include <pthread.h>


static id (*orig_objc_msgSend)(void);
static pthread_key_t _thread_key;


typedef struct {
    uintptr_t lr;
}call_stack;

typedef struct {
    call_stack *cs;
    uint index;
}common_data;


void before_objc_msgSend(id SELF, SEL _cmd, uintptr_t lr) {
    printf("before: \n");
    /// pthread_getspecific 获取指定key的同一线程中不同方法的共享数据
    common_data *_cd = (common_data *)pthread_getspecific(_thread_key);
    if (_cd == NULL) {
        _cd = (common_data *)malloc(sizeof(common_data));
        /// 调用栈
        _cd->cs = (call_stack *)malloc(sizeof(call_stack) * 1024);
        /// 当前调用栈索引
        _cd->index = 0;
        /// 存放共享数据
        pthread_setspecific(_thread_key, (void *)_cd);
    }
    /// 保存当前调用栈下一个函数方法执行地址
    _cd->cs[_cd->index].lr = lr;
    /// 调用栈前进
    _cd->index ++;
}
uintptr_t after_objc_msgSend() {
    printf("after: \n");
    common_data *_cd = (common_data *)pthread_getspecific(_thread_key);
    /// 后退
    _cd->index --;
    /// 获取即将完成的调用
    call_stack stack = _cd->cs[_cd->index];
    /// 将下一条函数指令返回,并存放到寄存器x0
    return stack.lr;
}
void release_stack_() {
    printf("release \n");
    common_data *_cd = (common_data *)pthread_getspecific(_thread_key);
    if(!_cd) return;
    if (_cd->cs) free(_cd->cs);
    free(_cd);
}
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
"stp x0, x1, [sp, #-16]!\n");

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
void start_hook() {
    printf("hook_start\n");
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        pthread_key_create(&_thread_key, &release_stack_);
        /// 替换系统的函数: 因为apple使用了系统函数动态绑定技术,所以可以hook系统函数,本地函数编译时地址已固定
        /// rebinding: 第一个参数,需要hook的系统函数名称
        ///            第二个参数,替换系统的函数
        ///            第三个参数,指向被hook函数,调用orig_objc_msgSend相当于调用原始函数
        rebind_symbols((struct rebinding[1]){ "objc_msgSend", hook_objc_msgSend, (void *)(&orig_objc_msgSend) }, 1);
    });
}
