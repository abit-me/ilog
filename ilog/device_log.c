//
//  device_log.c
//  ilog
//
//  Created by A on 2018/1/5.
//  Copyright © 2018年 A. All rights reserved.
//

#include "device_log.h"
#include "sds.h"

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/syslog_relay.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

static device_log_callback callback;

static int quit_flag = 0;

static char* cur_udid = NULL;

sds a_line_log;

static idevice_t device = NULL;
static syslog_relay_client_t syslog = NULL;

int device_log(const char *devid);

static void syslog_callback(char c, void *user_data)
{
    if (c != '\n') {
        a_line_log = sdscatprintf(a_line_log, "%c", c);
    } else {
        callback(a_line_log);
        sdsclear(a_line_log);
    }
}

static int start_logging(void)
{
    idevice_error_t ret = idevice_new(&device, cur_udid);
    if (ret != IDEVICE_E_SUCCESS) {
        fprintf(stderr, "Device with udid %s not found!?\n", cur_udid);
        return -1;
    }
    
    /* start and connect to syslog_relay service */
    syslog_relay_error_t serr = SYSLOG_RELAY_E_UNKNOWN_ERROR;
    serr = syslog_relay_client_start_service(device, &syslog, "idevicesyslog");
    if (serr != SYSLOG_RELAY_E_SUCCESS) {
        fprintf(stderr, "ERROR: Could not start service com.apple.syslog_relay.\n");
        idevice_free(device);
        device = NULL;
        return -1;
    }
    
    /* start capturing syslog */
    serr = syslog_relay_start_capture(syslog, syslog_callback, NULL);
    if (serr != SYSLOG_RELAY_E_SUCCESS) {
        fprintf(stderr, "ERROR: Unable tot start capturing syslog.\n");
        syslog_relay_client_free(syslog);
        syslog = NULL;
        idevice_free(device);
        device = NULL;
        return -1;
    }
    
    fprintf(stdout, "[connected]\n");
    fflush(stdout);
    
    return 0;
}

static void stop_logging(void)
{
    fflush(stdout);
    
    if (syslog) {
        syslog_relay_client_free(syslog);
        syslog = NULL;
    }
    
    if (device) {
        idevice_free(device);
        device = NULL;
    }
}

static void device_event_cb(const idevice_event_t* event, void* userdata)
{
    if (event->event == IDEVICE_DEVICE_ADD) {
        if (!syslog) {
            if (!cur_udid) {
                cur_udid = strdup(event->udid);
            }
            if (strcmp(cur_udid, event->udid) == 0) {
                if (start_logging() != 0) {
                    fprintf(stderr, "Could not start logger for udid %s\n", cur_udid);
                }
            }
        }
    } else if (event->event == IDEVICE_DEVICE_REMOVE) {
        if (syslog && (strcmp(cur_udid, event->udid) == 0)) {
            stop_logging();
            fprintf(stdout, "[disconnected]\n");
        }
    }
}

/**
 * signal handler function for cleaning up properly
 */
static void clean_exit(int sig)
{
    fprintf(stderr, "\nExiting...\n");
    quit_flag++;
}

void device_log_cb(const char *udid, device_log_callback cb) {
    
    callback = cb;
    a_line_log = sdsempty();
    device_log(udid);
}

int device_log(const char *udid) {
    
    signal(SIGINT, clean_exit);
    signal(SIGTERM, clean_exit);
    
    cur_udid = (char *)udid;
    
    idevice_event_subscribe(device_event_cb, NULL);
    
    while (!quit_flag) {
        sleep(1);
    }
    idevice_event_unsubscribe();
    stop_logging();
    
    if (udid) {
        free((void *)udid);
    }
    return 0;
}
