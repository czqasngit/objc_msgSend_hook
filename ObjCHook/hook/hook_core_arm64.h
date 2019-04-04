//
//  hook_core_arm64.h
//  ObjCHook
//
//  Created by legendry on 2019/4/1.
//  Copyright © 2019 legendry. All rights reserved.
//

#ifndef hook_core_arm64_h
#define hook_core_arm64_h

#include <stdio.h>
/// arm64
void arm64_start_objc_msgSend_hook(void);
void arm64_stop_objc_msgSend_hook(void);

#endif /* hook_core_arm64_h */
