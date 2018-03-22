//
//  device_id.c
//  ilog
//
//  Created by A on 2018/1/5.
//  Copyright © 2018年 A. All rights reserved.
//

#include "device_id.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>

CFMutableArrayRef get_device_ids()
{
    idevice_t device = NULL;
    lockdownd_client_t client = NULL;
    char **dev_list = NULL;
    char *device_name = NULL;
    int ret = 0;
    int count = 0;
    const char* udid = NULL;
    
    if (idevice_get_device_list(&dev_list, &count) < 0) {
        fprintf(stderr, "ERROR: Unable to retrieve device list!\n");
        return NULL;
    }
    
    CFMutableArrayRef arr = CFArrayCreateMutable(kCFAllocatorDefault, 0, NULL);
    
    for (int i = 0; dev_list[i] != NULL; i++) {
        udid = dev_list[i];
        idevice_new(&device, udid);
        if (!device) {
            fprintf(stderr, "ERROR: No device with UDID=%s attached.\n", udid);
            CFRelease(arr);
            return NULL;
        }
        
        if (LOCKDOWN_E_SUCCESS != lockdownd_client_new(device, &client, "idevice_id")) {
            idevice_free(device);
            fprintf(stderr, "ERROR: Connecting to device failed!\n");
            CFRelease(arr);
            return NULL;
        }
        
        if ((LOCKDOWN_E_SUCCESS != lockdownd_get_device_name(client, &device_name)) || !device_name) {
            fprintf(stderr, "ERROR: Could not get device name!\n");
            CFRelease(arr);
            return NULL;
        }
        
        lockdownd_client_free(client);
        idevice_free(device);
        
        if (ret == 0) {
            
            size_t result_len = strlen(device_name)+strlen(udid)+strlen(":");
            char *result = (char *)malloc(result_len);
            sprintf(result, "%s:%s", udid, device_name);
            CFArrayAppendValue(arr, (const void *)result);
        }
        
        if (device_name) {
            free(device_name);
        }
    }
    idevice_device_list_free(dev_list);
    
    return arr;
}

ideviceid id_from_arr(const CFMutableArrayRef arr, const CFIndex index) {
    
    char *sep_result[2];
    char *value = (char *)CFArrayGetValueAtIndex(arr, index);
    char *new_str = strdup(value);
    for (char ** ap = sep_result; (*ap = strsep(&new_str, ":")) != NULL;)
        if (**ap != '\0' && ++ap >= &sep_result[2]) break;
    
    ideviceid dev;
    dev.udid = sep_result[0];
    dev.name = sep_result[1];
    free(new_str);
    return dev;
}
