//
//  RegExpr.m
//  ilog
//
//  Created by A on 2018/3/22.
//  Copyright © 2018年 A. All rights reserved.
//

#import "RegExpr.h"

@interface RegExpr()
@property (nonatomic, strong) NSRegularExpression *regExpr;
@end

@implementation RegExpr

- (instancetype)init {
    
    if (self = [super init]) {
        
        // ([^\s]+\s+\d+\s+[^\s]+)\s+([^\s]+)\s+([^<]+)\s+([^\s]+)
        NSString *pattern = @"^([^\\s]+\\s+\\d+\\s+[^\\s]+)+\\s+([^\\s]+)\\s+([^<]+)\\s+([^\\s]+)";
        NSRegularExpressionOptions ops = NSRegularExpressionDotMatchesLineSeparators | NSRegularExpressionCaseInsensitive | NSRegularExpressionAnchorsMatchLines;
        NSError *err = nil;
        if (err) {
            printf("NSRegularExpression init err: %s\n", err.description.UTF8String);
            return nil;
        }
        self.regExpr = [NSRegularExpression regularExpressionWithPattern:pattern options:ops error:&err];
    }
    return self;
}

- (LogHead)getHead:(NSString *)log {
    
    __block LogHead head;
    memset(&head, 0x0, sizeof(LogHead));
    
    if (log == nil || [log length] == 0) {
        return head;
    }
    NSRange range = NSMakeRange(0, log.length);
    
    [self.regExpr enumerateMatchesInString:log options:NSMatchingReportProgress range:range usingBlock:^(NSTextCheckingResult * _Nullable result, NSMatchingFlags flags, BOOL * _Nonnull stop) {
        
        NSUInteger cnt = result.numberOfRanges;
        for (int i = 1; i < cnt; i++) {
            const char *str = [log substringWithRange:[result rangeAtIndex:i]].UTF8String;
            switch (i) {
                case 1:
                    head.date = str;
                    break;
                case 2:
                    head.device = str;
                    break;
                case 3:
                    head.process = str;
                    break;
                case 4:
                    head.level = str;
                    break;
                default:
                    break;
            }
        }
    }];
    return head;
}

@end

//NSString *logStr0 = @"Jan 20 01:12:44 swiftc iaptransportd[69] <Warning>: CIapPortAppleIDBus: Auth timer timeout completed on pAIDBPort:0x15c612140, portID:01 downstream port";
//NSString *logStr1 = @"Mar 15 15:37:35 jie com.apple.xpc.launchd[1] (org.hacx.hacxdaemon[7473]) <Notice>: Service exited due to signal: Killed: 9";
//NSString *logStr2 = @"Mar 22 18:23:29 iPhone kernel(corecapture)[0] <Notice>: 167854.627489 wlan0.A[18715] updateLinkQualityMetrics@2146:Report LQM to User Land 100, fAverageRSSI -70";
//NSString *logStr3 = @"Mar 22 19:21:52 jie com.apple.xpc.launchd[1] (org.hacx.hacxdaemon) <Notice>: Service only ran for 0 seconds. Pushing respawn out by 10 seconds.";
