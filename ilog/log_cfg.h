//
//  log_cfg.h
//  ilog
//
//  Created by A on 2018/3/23.
//  Copyright © 2018年 A. All rights reserved.
//

#ifndef log_cfg_h
#define log_cfg_h

typedef struct {
    bool show_date;             // if show date
    bool show_device;           // if show device name
    bool show_process;          // if show process name
    bool show_log_level;        // if show log level
} log_cfg_content;

typedef struct {
    char * show_by_ps_list[5];  // show log by process name list
    char * show_by_level;       // show log by log level
} log_cfg_filter;

typedef struct {
    char * include_keyword;     // show log by start with keyword
    char * exclude_keyword;     // don't show if start with keyword
} log_cfg_keyword;

typedef struct {
    log_cfg_content content;
    log_cfg_filter  filter;
    log_cfg_keyword keyword;
    bool            colored;     // if show log with color
} log_cfg;

#endif /* log_cfg_h */

//typedef struct {
//    bool coloed;              // if show log with color
//} log_cfg_color;
