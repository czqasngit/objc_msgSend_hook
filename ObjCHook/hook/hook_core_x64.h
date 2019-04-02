//
//  hook_core_x64.h
//  ObjCHook
//
//  Created by legendry on 2019/4/1.
//  Copyright © 2019 legendry. All rights reserved.
//

#ifndef hook_core_x64_h
#define hook_core_x64_h

#include <stdio.h>
/// intel x64 hook
void x64_start_objc_msgSend_hook(void);
void x64_stop_objc_msgSend_hook(void);
#endif /* hook_core_x64_h */
