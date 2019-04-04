//
//  main.m
//  ObjCHook
//
//  Created by legendry on 2019/3/26.
//  Copyright © 2019 legendry. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "AppDelegate.h"
#import "hook.h"
#import <objc/runtime.h>

int main(int argc, char * argv[]) {
    start_objc_msgSend_hook();
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
