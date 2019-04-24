//
//  ViewController.m
//  ObjCHook
//
//  Created by legendry on 2019/3/26.
//  Copyright © 2019 legendry. All rights reserved.
//

#import "ViewController.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    NSLog(@"............");
    [self test];
}
- (void)test {
//    [NSThread sleepForTimeInterval:1];
    NSLog(@"test");
    [self foo];
//    for (int i = 0 ; i < 800000; i ++) {
//        NSString *s = @"111";
//        int _st = s.integerValue;
//    }
    
}
- (void)foo {
    for (int i = 0 ; i < 1000000; i ++) {
        NSString *s = @"111";
        int _st = s.integerValue;
    }
}


@end
