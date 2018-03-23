//
//  RegExpr.h
//  ilog
//
//  Created by A on 2018/3/22.
//  Copyright © 2018年 A. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef struct
{
    const char *date;
    const char *device;
    const char *process;
    const char *level;
} LogHead;

typedef struct {
    
    LogHead     head;
    const char *content;
} Log;

@interface RegExpr : NSObject
- (Log)getLog:(NSString *)log;
@end
