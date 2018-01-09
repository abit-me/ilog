//
//  device_id.h
//  ilog
//
//  Created by A on 2018/1/5.
//  Copyright © 2018年 A. All rights reserved.
//

#ifndef device_id_h
#define device_id_h

#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>

typedef struct {
    char *udid;
    char *name;
} ideviceid;

CFMutableArrayRef get_device_ids();
ideviceid id_from_arr(CFMutableArrayRef arr, CFIndex index);
#endif /* device_id_h */
