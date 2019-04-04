//
//  hook_.h
//  ObjCHook
//
//  Created by legendry on 2019/3/28.
//  Copyright © 2019 legendry. All rights reserved.
//

#ifndef hook__h
#define hook__h

#include <stdio.h>

/// 开始hook
void start_objc_msgSend_hook(void);
/// 停止
void stop_objc_msgSend_hook(void);

#endif /* hook__h */
