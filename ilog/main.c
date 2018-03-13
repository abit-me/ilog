//
//  main.c
//  ilog
//
//  Created by A on 2018/1/4.
//  Copyright © 2018年 A. All rights reserved.
//

#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <regex.h>
#include <wchar.h>

#include "device_id.h"
#include "device_log.h"
#include "sds.h"
#include "color_log.h"

#define LINE_REGEX "(\\w+\\s+\\d+\\s+\\d+:\\d+:\\d+)\\s+(\\S+|)\\s+(\\w+)\\[(\\d+)\\]\\s+\\<(\\w+)\\>:\\s(.*)"

typedef struct {
    bool show_date_and_name;        // if show date and device name
    bool show_process_name;         // if show process name
    bool show_colored_log;          // if show log with color
    char * filter_in_ps_list[5];    // show log by process name list
    char * filter_in_keyword;       // show log by start with keyword
    char * filter_out_keyword;      // don't show if start with keyword
} log_cfg;

log_cfg cfg;

static int find_space_offsets(const char *buffer, size_t length, size_t *space_offsets_out)
{
    int o = 0;
    for (size_t i = 16; i < length; i++) {
        if (buffer[i] == ' ') {
            space_offsets_out[o++] = i;
            if (o == 3) {
                break;
            }
        }
    }
    return o;
}

static inline void write_fully(int fd, const char *buffer, size_t length)
{
    while (length) {
        ssize_t result = write(fd, buffer, length);
        if (result == -1)
            break;
        buffer += result;
        length -= result;
    }
}

static inline void write_string(int fd, const char *string)
{
    write_fully(fd, string, strlen(string));
}

static inline void write_const(int fd, const char * text)
{
    write_fully(fd, text, sizeof(text)-1);
}

void log_date_and_device_name(const char *buffer, size_t length, size_t space_offsets[3])
{
    
    write_const(STDOUT_FILENO, COLOR_DARK_WHITE);
    write_fully(STDOUT_FILENO, buffer, space_offsets[0]);
    printf("\n");
}

void log_process_name(const char *buffer, size_t length, size_t space_offsets[3])
{
    int pos = 0;
    for (int i = (int)space_offsets[0]; i < space_offsets[0]; i++) {
        if (buffer[i] == '[') {
            pos = i;
            break;
        }
    }
    write_const(STDOUT_FILENO, COLOR_CYAN);
    if (pos && buffer[space_offsets[1]-1] == ']') {
        write_fully(STDOUT_FILENO, buffer + space_offsets[0], pos - space_offsets[0]);
        write_const(STDOUT_FILENO, COLOR_DARK_CYAN);
        write_fully(STDOUT_FILENO, buffer + pos, space_offsets[1] - pos);
    } else {
        write_fully(STDOUT_FILENO, buffer + space_offsets[0], space_offsets[1] - space_offsets[0]);
    }
    printf("\n");
}

void log_log_level(const char *buffer, size_t length, size_t space_offsets[3])
{
    size_t levelLength = space_offsets[2] - space_offsets[1];
    if (levelLength > 4) {
        char *normalColor;
        char *darkColor;
        if (levelLength == 9 && memcmp(buffer + space_offsets[1], " <Debug>:", 9) == 0){
            normalColor = COLOR_MAGENTA;
            darkColor = COLOR_DARK_MAGENTA;
        } else if (levelLength == 11 && memcmp(buffer + space_offsets[1], " <Warning>:", 11) == 0){
            normalColor = COLOR_YELLOW;
            darkColor = COLOR_DARK_YELLOW;
        } else if (levelLength == 9 && memcmp(buffer + space_offsets[1], " <Error>:", 9) == 0){
            normalColor = COLOR_RED;
            darkColor = COLOR_DARK_RED;
        } else if (levelLength == 10 && memcmp(buffer + space_offsets[1], " <Notice>:", 10) == 0) {
            normalColor = COLOR_GREEN;
            darkColor = COLOR_DARK_GREEN;
        } else {
            goto level_unformatted;
        }
        write_string(STDOUT_FILENO, darkColor);
        write_fully(STDOUT_FILENO, buffer + space_offsets[1], 2);
        write_string(STDOUT_FILENO, normalColor);
        write_fully(STDOUT_FILENO, buffer + space_offsets[1] + 2, levelLength - 4);
        write_string(STDOUT_FILENO, darkColor);
        write_fully(STDOUT_FILENO, buffer + space_offsets[1] + levelLength - 2, 1);
        write_const(STDOUT_FILENO, COLOR_DARK_WHITE);
        write_fully(STDOUT_FILENO, buffer + space_offsets[1] + levelLength - 1, 1);
    } else {
    level_unformatted:
        write_const(STDOUT_FILENO, COLOR_RESET);
        write_fully(STDOUT_FILENO, buffer + space_offsets[1], levelLength);
    }
    printf("\n");
}

void log_content(const char *buffer, size_t length, size_t space_offsets[3])
{
    write_const(STDOUT_FILENO, COLOR_RESET);
    write_fully(STDOUT_FILENO, buffer + space_offsets[2], length - space_offsets[2]);
    printf("\n");
}

sds get_process_name(sds log) {
    
//    size_t space_offsets[3];
//    size_t length = sdslen(log);
//    const char *buffer = log;
//    int o = find_space_offsets(buffer, length, space_offsets);
//    printf("%d", o);
    printf("start regex\n");
    regex_t reg;
    char regex[] = LINE_REGEX;
    size_t nmatch = 1;
    regmatch_t pmatch[1];
    char errbuf[100];
    int re;
    
    // Compile regex to reg
    re = regcomp(&reg, regex, REG_EXTENDED | REG_NEWLINE);
    if (0 != re) {
        regerror(re, &reg, errbuf, 100);
        printf("regcomp err: %s\n", errbuf);
        return NULL;
    }
    
    // Use reg which compiled from pattern to match substring in whole string str
    re = regexec(&reg, log, nmatch, pmatch, 0);
    if (REG_NOMATCH == re) {
        printf("No match\n");
        return NULL;
    } else if (0 == re) {
        printf("matched:\n");
        regoff_t i;
        for (i = pmatch[0].rm_so; i < pmatch[0].rm_eo; ++i) {
            putchar(log[i]);
        }
        printf("\n");
        printf("%lld ====== %lld\n", pmatch[0].rm_so, pmatch[0].rm_eo);
        return NULL;
    }
    
    //Free &reg used in recomp
    regfree(&reg);
    printf("end regex\n");
    return NULL;
}

/*

 正则表达式示例表
 字 符 意 义 示 例
 * 任意长度的字符串。 a* 表示: 空字符串、aaaa、a…
 ? 长度为0或者1的字符串。 a? 表示: 空字符串和a。
 + 长度为一个或者多个的字符串。 a+表示:a、aa、aaaaaa…
 . 任意字符。 a. 表示:a后跟任意字符。
 {} 代表上一规则重复数目、
 {1,1,s}包含一组匹配花括号，里面有两个数字和一个字符，表示在指定次数范围内找到字符。 a{3}表示:三个a、
 a{1,3}表示:一个到三个a、
 a{3,} 表示:大于等于三个a、
 {3，7，a}表示在3到7次重复范围内匹配字符a。
 [] 集合,代表方括号中任意一个字符。 [ab] 表示:a或者b都可以、
 [a-z] 表示:从a到z的字符。
 () 组,代表一组字符。 (ab){2}表示:abab。
 a/b 同时满足。 a/b表示:字符串a后跟字符串b才能满足要求。
 a|b 并列,代表符合a或者符合b都可以 a|b表示: 字符串a或者字符串b都满足要求。
 ^ 如果放在开头表示代表该规则必须在字符串的开头，其他位置代表字符本身。
 如果放在[]中的开头表示对该集合取反,其他位置代表字符本身。 ^a表示:a必须在字符串的开头、
 [^a]表示:除了a以外的其他字符。
 $ 如果放在最后表示该规则必须放在最后,其他位置代表字符本身。 a$表示:a必须在字符串最后。
 /:s 正则表达式用 /:s 表示空格。 a/:sb 匹配 a b。
 /:a 正则表达式用 /:a 表示字符与数字。 a/:a 匹配 ab、a6 等。
 /:c 正则表达式用 /:c 仅表示字符。 a/:c 匹配 ac等，不匹配a1等。
 /:p 正则表达式用 /:p 表示可打印字符。
 /:D 正则表达式用 /:d 仅表示数字。 a/:c 匹配 a1等，不匹配ac等。
 /:x00 正则表达式用 /:x00 表示ASCII字符。
 /:r 正则表达式用 /:r 表示回车。
 /:N 正则表达式用 /:d 表示换行。
 
*/
void test_regex()
{
    const char *log = "Jan 20 01:12:44 swiftc iaptransportd[69] <Warning>: CIapPortAppleIDBus: Auth timer timeout completed on pAIDBPort:0x15c612140, portID:01 downstream port";
    regex_t reg;
    char regex[] = "(\\w+\\s+\\d+\\s+\\d+:\\d+:\\d+)\\s+(\\S+|)\\s+(\\w+)\\[(\\d+)\\]\\s+\\<(\\w+)\\>:\\s(.*)";
    size_t nmatch = 1;
    regmatch_t pmatch[1];
    char errbuf[100];
    int re;
    
    // Compile regex to reg
    //re = regcomp(&reg, regex, REG_EXTENDED |cREG_NEWLINE);
    re = regcomp(&reg, regex, REG_NEWLINE | REG_NOSUB);
    if (0 != re) {
        regerror(re, &reg, errbuf, 100);
        printf("regcomp err: %s\n", errbuf);
        return;
    }
    
    // Use reg which compiled from pattern to match substring in whole string str
    re = regexec(&reg, log, nmatch, pmatch, 0);
    if (REG_NOMATCH == re) {
        printf("No match\n");
        return;
    } else if (0 == re) {
        printf("matched:\n");
        regoff_t i;
        for (i = pmatch[0].rm_so; i < pmatch[0].rm_eo; ++i) {
            putchar(log[i]);
        }
        printf("\n");
        printf("%lld ====== %lld\n", pmatch[0].rm_so, pmatch[0].rm_eo);
        return;
    }
    
    //Free &reg used in recomp
    regfree(&reg);
}

void handle_log(sds log)
{
    if (cfg.show_colored_log) {
        size_t space_offsets[3];
        size_t length = sdslen(log);
        const char *buffer = log;
        int o = find_space_offsets(buffer, length, space_offsets);

        if (o == 3) {

            // Log date and device name
            if (cfg.show_date_and_name) {
                log_date_and_device_name(buffer, length, space_offsets);
            }

            // Log process name
            if (cfg.show_process_name) {
                log_process_name(buffer, length, space_offsets);
            }

            // Log level
            log_log_level(buffer, length, space_offsets);

            // Log content
            log_content(buffer, length, space_offsets);
        } else {
            //write_fully(STDOUT_FILENO, buffer, length);
        }
    } else {
        
        printf("%s\n", log);
        //printf("%s\n", log);
        //get_process_name(log);

//        size_t keyword_len = strlen(cfg.filter_in_keyword);
//        if (memcmp(log, cfg.filter_in_keyword, keyword_len) == 0) {
//            printf("%s\n", log);
//        }

//        if (strstr(log, "xxooxxooxxoo = ") != 0) {
//            wprintf("%s\n", (wchar_t *)log);
//        }

        //if (strstr(log, "xxooxxooxxoo = ") != 0) {
//            wprintf(L"%s\n", (wchar_t *)log);
        //}

    }
    
//    const char * process_name = get_process_name(log);
//    if (strstr(process_name, cfg.filter_ps_list[0]) != NULL) {
//        printf("%s\n", log);
//    }
}

void log_callback(sds log)
{
    handle_log(log);
}

int main(int argc, const char * argv[])
{
//    char *local = setlocale(LC_ALL, "");
//    printf("local: %s\n", local);

    test_regex();
    return 0;
    
    cfg.show_date_and_name = true;
    cfg.show_process_name = true;
    cfg.show_colored_log = false;
    cfg.filter_in_keyword = "xxx";
    cfg.filter_out_keyword = "ooo";
    cfg.filter_in_ps_list[0] = "SpringBoard";
    cfg.filter_in_ps_list[1] = "com.mobyapps.watchdogd";
    
    CFMutableArrayRef arr = get_device_ids();
    
    if (arr == NULL) {
        printf("find iOS device failed\n");
        return 1;
    }

    CFIndex count = CFArrayGetCount(arr);
    if (count == 0) {
        printf("no iOS device connected\n");
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
        printf("choose an iOS device\n");
        printf(COLOR_RESET);
        int c = 0;
        scanf("%d", &c);
        if (c < 0 || c >= count) {
            printf(COLOR_RED);
            printf("choose err\n");
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
