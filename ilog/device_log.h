//
//  device_log.h
//  ilog
//
//  Created by A on 2018/1/5.
//  Copyright © 2018年 A. All rights reserved.
//

#ifndef device_log_h
#define device_log_h

#include <stdio.h>
#include "sds.h"
typedef void (*device_log_callback)(sds log);
void device_log_cb(const char *udid, device_log_callback cb);
#endif /* device_log_h */
