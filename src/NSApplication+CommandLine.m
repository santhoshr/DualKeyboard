#import "NSApplication+CommandLine.h"
#import <unistd.h>

@implementation NSApplication (CommandLine)

- (BOOL)isRunningFromCommandLine {
    return isatty(STDIN_FILENO);
}

@end
