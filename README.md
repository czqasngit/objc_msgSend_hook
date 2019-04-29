## 精简详解,完整核心功能

![](https://ws3.sinaimg.cn/large/006tNc79gy1g2j825epgoj31jk0dw4qp.jpg)

## 1.汇编入门:
    10分钟入门
        - http://blackteachinese.com/2017/07/12/arm64/
    iOS开发汇编入门
        - https://blog.cnbluebox.com/blog/2017/07/24/arm64-start/?hmsr=toutiao.io&utm_medium=toutiao.io&utm_source=toutiao.io
    小例子入门
        - https://juejin.im/post/5aabcae1f265da238d507a68
    理解 iOS ARM
        - https://www.jianshu.com/p/544464a5e630?utm_campaign=maleskine&utm_content=note&utm_medium=seo_notes&utm_source=recommendation

## 2.Mach-O 理解:
    Mach-O文件结构
        - https://hawk0620.github.io/blog/2018/03/22/study-mach-o-file/
    探秘Mach-O
        - https://www.jianshu.com/p/1f22d1e667e3

## 3. fishhook
    - https://www.jianshu.com/p/4d86de908721

## 4. objc_msgSend hook 完整精简代码(详细的注释)

### `hook_core_arm64.c`实现了对 arm64架构的objc_msgSend hook

```

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

```

### `hook_core.c` 实现了对调用堆栈的统计与打印

```
//
//  hook_core.c
//  ObjCHook
//
//  Created by legendry on 2019/4/1.
//  Copyright © 2019 legendry. All rights reserved.
//

#include "hook_core.h"
#include "hook_utils.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <objc/runtime.h>
#include <dispatch/dispatch.h>
#include "fishhook.h"

static pthread_key_t _thread_key;

typedef struct {
    uintptr_t lr;
    SEL _cmd;
    /// 开始时间
    uint64_t start_time;
    /// 结束时间
    uint64_t end_time;
    /// 类名
    char *class_name;
    /// 函数名
    char *method_name;
    /// 调用深度
    uint8_t dept;
    /// 是否是类方法
    bool is_class_method;
}call_stack;

typedef struct {
    /// 调用栈
    call_stack *cs;
    /// 调用栈第一个: 每一个objc_msgSend开始
    call_stack *first;
    uint index;
    /// 当前数据总大小
    uint size;
    /// 总开销
    uint64_t cost;
    /// 是否是主线程
    bool is_main_thread;
    // 调用栈信息
    char *stack_info;
    uint64_t stack_info_size;
}common_data;

///获取线程共享数据
common_data *get_thread_call_stack() {
    /// pthread_getspecific 获取指定key的同一线程中不同方法的共享数据
    common_data *_cd = (common_data *)pthread_getspecific(_thread_key);
    if (_cd == NULL) {
        _cd = (common_data *)malloc(sizeof(common_data));
        _cd->size = 10;
        /// 调用栈
        _cd->cs = (call_stack *)malloc(sizeof(call_stack) * _cd->size);
        /// 当前调用栈索引
        _cd->index = 0;
        _cd->cost = 0;
        /// 存放共享数据
        pthread_setspecific(_thread_key, (void *)_cd);
    }
    _cd->is_main_thread = pthread_main_np();
    return _cd;
}
/// 如果调用栈大小不够,重新分配空间
void realloc_call_stack_memery_ifneed() {
    common_data *_cd = get_thread_call_stack();
    /// 重新分配空间
    if (_cd->index >= _cd->size) {
        _cd->size += 10;
        _cd->cs = (call_stack *)realloc(_cd->cs, sizeof(call_stack) * _cd->size);
    }
}
/// 打印调用信息
char *dump_call_stack(call_stack *_cs) {
    uint64_t cost = _cs->end_time - _cs->start_time;
    if (cost > 100 && !hook_has_prefix(_cs->class_name, "OS_")) {
        int _count = _cs->dept;
        while (_count > 0) {
            printf(" ");
            _count --;
        }
        char *space = (char *)malloc(_cs->dept);
        memset(space, ' ', _cs->dept);
        char *str = (char *)malloc(1024);
        sprintf(str, "%s %c [%s %s] %lld ms\n", space, '-', _cs->class_name, _cs->method_name, cost);
        free(space);
        return str;
    }
    return NULL;
}
/// 打印一个objc_msgSend完整调用信息
void dump_method() {
    common_data *_cd = get_thread_call_stack();
    _cd->cost = _cd->first->end_time - _cd->first->start_time;
    if(_cd->cost > 400 && _cd->first && !hook_has_prefix(_cd->first->class_name, "OS_")) {
        printf("[%s][%s: %s]: %lld ms\n", _cd->is_main_thread ? "主" : "子", _cd->first->class_name, _cd->first->method_name, _cd->cost);
        printf("%s \n", _cd->stack_info);
    }
    
}
/// hook 之前
void before_objc_msgSend(id object, SEL _cmd, uintptr_t lr) {
    realloc_call_stack_memery_ifneed();
    common_data *_cd = get_thread_call_stack();
    /// 保存当前调用栈下一个函数方法执行地址
    call_stack *_cs =  &(_cd->cs[_cd->index]);
    /// 正常执行程序的下一条指令,保存起来在hook完之后需要恢复到hook前的状态,程序lr寄存器能正常执行
    _cs->lr = lr;
    /// 保存执行的SEL
    _cs->_cmd = _cmd;
    /// 保存执行前的时间
    _cs->start_time = hook_getMillisecond();
    /// 保存当前调用栈的深度
    _cs->dept = _cd->index;
    /// 保存当前对象的类名
    _cs->class_name = (char *)hook_getObjectClassName(object);
    /// 保存当前调用的方法名
    _cs->method_name = (char *)_cs->_cmd;
    /// 判断是否是元类的实例方法,即类的类方法
//    Class __cls = object_getClass(object);
//    _cs->is_class_method = class_isMetaClass(__cls);
    /// index为0表示调用栈顶,即objc_msgSend初始调用(objc_msgSend内部还会调用其它objc_msgSend)
    if(_cd->index == 0) {
        _cd->first = _cs;
        _cd->stack_info_size = 1024;
        if(_cd->stack_info) free(_cd->stack_info);
        _cd->stack_info = (char *)malloc(1024);
    }
    /// 入栈
    /// 调用栈前进
    _cd->index ++;
}
/// hook 之后
uintptr_t after_objc_msgSend() {
    common_data *_cd = (common_data *)pthread_getspecific(_thread_key);
    /// 后退
    _cd->index --;
    /// 获取即将完成的调用
    call_stack *stack = &(_cd->cs[_cd->index]);
    stack->end_time = hook_getMillisecond();
//    _cd->cost += (stack->end_time - stack->start_time);
    /// 打印单个方法执行信息
    if(_cd->index > 0) {
        char *_stack_info = dump_call_stack(stack);
        if(_stack_info) {
            uint64_t _stack_info_count = strlen(_stack_info);
            uint64_t cur_stack_info_size = strlen(_cd->stack_info);
            // 判断是否超出当前容量
            if(_stack_info_count + cur_stack_info_size > _cd->stack_info_size) {
                //扩容
                _cd->stack_info_size = _stack_info_count + cur_stack_info_size + 1024;
                _cd->stack_info = (char *)realloc(_cd->stack_info, _cd->stack_info_size);
            }
            memcpy(_cd->stack_info + cur_stack_info_size, _stack_info, _stack_info_count);
            free(_stack_info);
        }
    }
    /// 一个函数调用完成
    /// 打印所有方法执行信息
    if (_cd->index == 0){
        dump_method();
        _cd->stack_info_size = 1024;
        if(_cd->stack_info) free(_cd->stack_info);
        _cd->stack_info = (char *)malloc(_cd->stack_info_size);
    }
    /// 将下一条函数指令返回,并存放到寄存器x0
    return stack->lr;
}
/// 释放与线程共享数据的相关资源
void release_thread_stack() {
    printf("release \n");
    common_data *_cd = (common_data *)pthread_getspecific(_thread_key);
    if(!_cd) return;
    if (_cd->cs) free(_cd->cs);
    if (_cd->stack_info) free(_cd->stack_info);
    free(_cd);
}

void start_hook(void *hook_objc_msgSend, void **origin_objc_msgSend) {
    printf("hook_start\n");
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        pthread_key_create(&_thread_key, &release_thread_stack);
        /// 替换系统的函数: 因为apple使用了系统函数动态绑定技术,所以可以hook系统函数,本地函数编译时地址已固定
        /// rebinding: 第一个参数,需要hook的系统函数名称
        ///            第二个参数,替换系统的函数
        ///            第三个参数,指向被hook函数,调用orig_objc_msgSend相当于调用原始函数
        rebind_symbols((struct rebinding[1]){ "objc_msgSend", hook_objc_msgSend, origin_objc_msgSend}, 1);
    });
}

```

