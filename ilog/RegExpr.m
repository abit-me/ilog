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

- (Log)getLog:(NSString *)logStr {
    
    __block Log log;
    memset(&log, 0x0, sizeof(log));
    
    if (logStr == nil || [logStr length] == 0) {
        return log;
    }
    NSRange range = NSMakeRange(0, logStr.length);
    
    NSArray<NSTextCheckingResult *> * results = [self.regExpr matchesInString:logStr options:NSMatchingReportProgress range:range];
    
    for (int i = 0; i < results.count; ++i) {
        
        NSTextCheckingResult *result = results[i];
        NSUInteger cnt = result.numberOfRanges;
        NSRange logHeadRng = result.range;
        NSString *logHead = [logStr substringWithRange:logHeadRng];
        NSRange logContentRng = NSMakeRange(logHeadRng.length+1, [logStr length]-logHeadRng.length-1);
        NSString *logContent = [logStr substringWithRange:logContentRng];
//        printf("h:%s\n", logHead.UTF8String);
//        printf("c:%s\n", logContent.UTF8String);
        
        log.content = logContent.UTF8String;
        
        LogHead head;
        memset(&head, 0x0, sizeof(LogHead));
        
        for (int j = 0; j < cnt; ++j) {
            NSRange subRng = [result rangeAtIndex:j];
            const char *subStr = [logHead substringWithRange:subRng].UTF8String;
            //printf("%d == %s\n", j, subStr.UTF8String);
            switch (j) {
                case 1:
                    head.date = subStr;
                    break;
                case 2:
                    head.device = subStr;
                    break;
                case 3:
                    head.process = subStr;
                    break;
                case 4:
                    head.level = subStr;
                    break;
                default:
                    break;
            }
        }
        log.head = head;
    }
    return log;
}

@end

//NSString *logStr0 = @"Jan 20 01:12:44 swiftc iaptransportd[69] <Warning>: CIapPortAppleIDBus: Auth timer timeout completed on pAIDBPort:0x15c612140, portID:01 downstream port";
//NSString *logStr1 = @"Mar 15 15:37:35 jie com.apple.xpc.launchd[1] (org.hacx.hacxdaemon[7473]) <Notice>: Service exited due to signal: Killed: 9";
//NSString *logStr2 = @"Mar 22 18:23:29 iPhone kernel(corecapture)[0] <Notice>: 167854.627489 wlan0.A[18715] updateLinkQualityMetrics@2146:Report LQM to User Land 100, fAverageRSSI -70";
//NSString *logStr3 = @"Mar 22 19:21:52 jie com.apple.xpc.launchd[1] (org.hacx.hacxdaemon) <Notice>: Service only ran for 0 seconds. Pushing respawn out by 10 seconds.";
