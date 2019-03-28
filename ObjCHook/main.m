//
//  main.m
//  ObjCHook
//
//  Created by legendry on 2019/3/26.
//  Copyright © 2019 legendry. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "AppDelegate.h"
#import "hook_.h"

int main(int argc, char * argv[]) {
    start_hook();
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
