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

@interface RegExpr : NSObject
- (LogHead)getHead:(NSString *)log;
@end
