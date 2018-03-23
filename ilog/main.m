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
#include "log_color.h"
#include "log_cfg.h"

RegExpr *reg;
log_cfg cfg;
const char *last_ps;

bool if_ps_in_cfg_ps_list(const char *str) {
    
    NSString *ps = [NSString stringWithUTF8String:str];
    for (int i = 0; cfg.filter.show_by_ps_list[i] != NULL; ++i) {
        NSString *cfgPs = [NSString stringWithUTF8String:cfg.filter.show_by_ps_list[i]];
        if ([ps hasPrefix:cfgPs]) {
            return true;
        }
    }
    return false;
}

void handle_log(sds clog)
{
    NSString *logStr = [NSString stringWithUTF8String:clog];
    
    if (logStr == nil || [logStr length] == 0) {
        printf("%s\n", logStr.UTF8String);
        return;
    }
    
    Log log = [reg getLog:logStr];
    LogHead head = log.head;
    
    if (cfg.filter.show_by_ps_list[0] != nil) { // have process filter
        if (head.process != nil && !if_ps_in_cfg_ps_list(head.process)) {
            // process not in the ps list
            //printf("%s is not in the cfg list\n", head.process);
            last_ps = head.process;
            return;
        }
    }
    
    // "        register = 1;" this log in process of SpringBoard
    if (head.date == nil || head.device == nil || head.process == nil || head.level == nil) {
        
        //printf("+++++++:%s\n", log.content);
        if (log.content == nil) { // means it is not a compelete log, appendix of last_ps
            if (if_ps_in_cfg_ps_list(last_ps)) {
                printf("-------:%s\n", clog);
            } else {
                //printf("I catch you!\n");
                return;
            }
        }
        return;
    }
    
    if (cfg.content.show_date && head.date != nil) {
        if (cfg.colored) {
            printf(COLOR_GREEN);
            printf("%s ", head.date);
            printf(COLOR_RESET);
        } else {
            printf("%s ", head.date);
        }
    }
    
    if (cfg.content.show_device && head.device != nil) {
        if (cfg.colored) {
            printf(COLOR_DARK_GREEN);
            printf("%s ", head.device);
            printf(COLOR_RESET);
        } else {
            printf("%s ", head.device);
        }
    }
    
    if (cfg.content.show_process && head.process != nil) {
        if (cfg.colored) {
            printf(COLOR_RED);
            printf("%s ", head.process);
            printf(COLOR_RESET);
        } else {
            printf("%s ", head.process);
        }
    }
    
    if (cfg.content.show_log_level && head.level != nil) {
        if (cfg.colored) {
            printf(COLOR_CYAN);
            printf("%s ", head.level);
            printf(COLOR_RESET);
        } else {
            printf("%s ", head.level);
        }
    }
    if (log.content) {
        printf("%s\n", log.content);
    }
}

void log_callback(sds log)
{
    @autoreleasepool { // prevent memory leaking
        
        handle_log(log);
    }
}

void set_default_log_cfg() {
    
    memset(&cfg, 0x0, sizeof(log_cfg));
    
    cfg.colored = true;
    cfg.content.show_date = false;
    cfg.content.show_device = false;
    cfg.content.show_process = true;
    cfg.content.show_log_level = true;
    cfg.filter.show_by_level = "<Debug>";
    //cfg.filter.show_by_ps_list[0] = "SpringBoard";
    cfg.filter.show_by_ps_list[0] = "kernel";
    cfg.filter.show_by_ps_list[1] = "CommCenter";
    cfg.filter.show_by_ps_list[2] = "locationd";
    cfg.filter.show_by_ps_list[3] = "iDoll";
    cfg.keyword.include_keyword = "xxx";
    cfg.keyword.exclude_keyword = "ooo";
}

int main(int argc, const char * argv[]) {
    
    set_default_log_cfg();
    reg = [RegExpr new];
    
    CFMutableArrayRef arr = get_device_ids();
    
    if (arr == NULL) {
        printf("Find iOS device failed\n");
        return 1;
    }
    
    CFIndex count = CFArrayGetCount(arr);
    if (count == 0) {
        printf("No iOS device connected\n");
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
        printf("Choose an iOS device\n");
        printf(COLOR_RESET);
        int c = 0;
        scanf("%d", &c);
        if (c < 0 || c >= count) {
            printf(COLOR_RED);
            printf("Choose err\n");
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
