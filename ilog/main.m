//
//  main.m
//  ilog
//
//  Created by A on 2018/3/14.
//  Copyright © 2018年 A. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>
#import "RegExpr.h"

#include <stdio.h>
#include <regex.h>

#include "device_id.h"
#include "device_log.h"
#include "sds.h"
#include "color_log.h"

RegExpr *reg;


void log_callback(sds log)
{
//    printf(COLOR_GREEN);
//    printf("%s\n", log);
//    printf(COLOR_RESET);
    
    @autoreleasepool { // prevent memory leaking
        
        NSString *logStr = [NSString stringWithUTF8String:log];
        LogHead head = [reg getHead:logStr];
        if (head.date != nil && head.device != nil && head.process != nil && head.level != nil) {
            printf("head.date:      %s\n", head.date);
            printf("head.device:    %s\n", head.device);
            printf("head.process:   %s\n", head.process);
            printf("head.level:     %s\n", head.level);
        }
    }
}

int main(int argc, const char * argv[]) {
    
    reg = [RegExpr new];
    
    CFMutableArrayRef arr = get_device_ids();
    
    if (arr == NULL) {
        printf("find iOS device failed\n");
        return 1;
    }
    
    CFIndex count = CFArrayGetCount(arr);
    if (count == 0) {
        printf("no iOS device connected\n");
        CFRelease(arr);
        return 2;
    } else if (count == 1) {
        ideviceid ret_idevid = id_from_arr(arr, 0);
        device_log_cb(ret_idevid.udid, log_callback);
    } else if (count >=2) {
        
        printf(COLOR_BLUE);
        for (int i = 0; i < count; ++i) {
            ideviceid ret_idevid = id_from_arr(arr, i);
            printf("%d : %s -> %s\n", i, ret_idevid.udid, ret_idevid.name);
        }
        
        printf(COLOR_GREEN);
        printf("choose an iOS device\n");
        printf(COLOR_RESET);
        int c = 0;
        scanf("%d", &c);
        if (c < 0 || c >= count) {
            printf(COLOR_RED);
            printf("choose err\n");
            printf(COLOR_RESET);
            exit(3);
        } else {
            ideviceid c_id = id_from_arr(arr, c);
            device_log_cb(c_id.udid, log_callback);
        }
    }
    
    CFRelease(arr);
    //CFRunLoopRun();
    return 0;
}
