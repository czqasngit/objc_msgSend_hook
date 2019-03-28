# `objc_msgSend` Hook 精简学习过程

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

```

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


```

