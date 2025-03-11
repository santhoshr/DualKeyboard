#import "DualManager+KeyboardStatus.h"
#import "DualManager+MenuBar.h"
#import "DualManager+ConsoleWindow.h"
#import "NSApplication+CommandLine.h"

@implementation DualManager (KeyboardStatus)

- (void)setupStatusBar {
    if (!self.debugMode) return;
    
    NSString *startMsg = @"Dual Start\n";
    NSString *statusMsg = [NSString stringWithFormat:@"Current Mode: %c | Debug: %s\n", 
                          self.currentMode, self.debugMode ? "ON" : "OFF"];
    
    if ([NSApp isRunningFromCommandLine]) {
        printf("\033[1;36m%s\033[0m", [startMsg UTF8String]);
        printf("%s", [statusMsg UTF8String]);
    } else {
        [self appendToConsole:startMsg];
        [self appendToConsole:statusMsg];
    }
}

- (void)updateStatusWithMode:(char)mode {
    if (mode != 'I' && mode != 'N') return;
    if (mode == self.currentMode) return;
    
    self.currentMode = mode;
    [self updateMenuBarStatus];
    
    if (self.debugMode) {
        NSString *modeMsg = [NSString stringWithFormat:@"Mode Changed -> Current: %c | Debug: %s\n",
                            self.currentMode, self.debugMode ? "ON" : "OFF"];
        
        if ([NSApp isRunningFromCommandLine]) {
            printf("\033[1;33m%s\033[0m", [modeMsg UTF8String]);
        } else {
            [self appendToConsole:modeMsg];
        }
    }
}

- (void)cleanupStatusBar {
    if (!self.debugMode) return;
    printf("\n\033[1;31mDual Keyboard Exit\033[0m\n");
    printf("Final Mode: \033[1;32m%c\033[0m | Debug: \033[1;33m%s\033[0m\n",
           self.currentMode, self.debugMode ? "ON" : "OFF");
    fflush(stdout);
}

@end