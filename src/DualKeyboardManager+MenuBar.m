#import "DualKeyboardManager+MenuBar.h"
#import "DualKeyboardManager+KeyboardMapping.h"
#import <AppKit/AppKit.h>

@implementation DualKeyboardManager (MenuBar)

- (void)setupMenuBar {
    self.statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
    self.statusItem.button.title = @"⌨️ I";
    
    self.statusMenu = [[NSMenu alloc] init];
    
    // Create mode menu item with a fixed tag for easy lookup
    NSMenuItem *modeItem = [[NSMenuItem alloc] initWithTitle:@"Mode: Insert" 
                                                     action:nil 
                                              keyEquivalent:@""];
    modeItem.tag = 1001;
    [self.statusMenu addItem:modeItem];
    [self.statusMenu addItem:[NSMenuItem separatorItem]];
    
    if (self.debugMode) {
        NSMenuItem *debugItem = [[NSMenuItem alloc] initWithTitle:[NSString stringWithFormat:@"Debug: %@", 
                                                                  self.debugMode ? @"ON" : @"OFF"] 
                                                         action:@selector(toggleDebugMode) 
                                                  keyEquivalent:@""];
        debugItem.tag = 1002;
        [debugItem setTarget:self];
        [self.statusMenu addItem:debugItem];
        [self.statusMenu addItem:[NSMenuItem separatorItem]];
    }
    
    NSMenuItem *exitItem = [[NSMenuItem alloc] initWithTitle:@"Exit" 
                                                    action:@selector(exitApplication) 
                                             keyEquivalent:@""];
    exitItem.tag = 1003;
    [exitItem setTarget:self];
    [self.statusMenu addItem:exitItem];
    
    self.statusItem.menu = self.statusMenu;
    
    // Initial update
    [self updateMenuBarStatus];
}

- (void)updateMenuBarStatus {
    if (!self.statusItem) return;
    
    dispatch_async(dispatch_get_main_queue(), ^{
        // Update menu bar icon
        NSString *modeTitle = [NSString stringWithFormat:@"⌨️ %c", self.currentMode];
        self.statusItem.button.title = modeTitle;
        
        // Update mode menu item
        NSMenuItem *modeItem = [self.statusMenu itemWithTag:1001];
        if (modeItem) {
            modeItem.title = [NSString stringWithFormat:@"Mode: %@", 
                             self.currentMode == 'I' ? @"Insert" : @"Navigation"];
        }
        
        // Update debug menu item if it exists
        NSMenuItem *debugItem = [self.statusMenu itemWithTag:1002];
        if (debugItem) {
            debugItem.title = [NSString stringWithFormat:@"Debug: %@", 
                             self.debugMode ? @"ON" : @"OFF"];
        }
        
        // Ensure exit item remains unchanged
        NSMenuItem *exitItem = [self.statusMenu itemWithTag:1003];
        if (exitItem) {
            exitItem.title = @"Exit";
            exitItem.action = @selector(exitApplication);
            exitItem.target = self;
        }
    });
}

- (void)toggleDebugMode {
    self.debugMode = !self.debugMode;
    [self updateMenuBarStatus];
    
    printf("\nDebug messages %s\n", self.debugMode ? "enabled" : "disabled");
    fflush(stdout);
}

- (void)exitApplication {
    [self cleanup];
    exit(0);
}

@end