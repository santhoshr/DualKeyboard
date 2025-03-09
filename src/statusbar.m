#import <Cocoa/Cocoa.h>
#include "../include/menubar.h"

@interface StatusBarDelegate : NSObject <NSApplicationDelegate>
- (void)toggleDebug:(id)sender;
- (void)exitApp:(id)sender;
@end

static NSStatusItem *statusItem = nil;
static NSMenu *statusMenu = nil;
static bool debugEnabled = false;
static bool shouldKeepRunning = true;
static NSThread *uiThread = nil;
static StatusBarDelegate *delegate = nil;

@implementation StatusBarDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    printf("ApplicationDidFinishLaunching called\n");
    dispatch_async(dispatch_get_main_queue(), ^{
        create_status_bar();
    });
}

- (void)toggleDebug:(id)sender {
    printf("Toggle debug called\n");
    debugEnabled = !debugEnabled;
    debug_mode = debugEnabled;
    NSMenuItem *debugItem = [statusMenu itemWithTag:1];
    [debugItem setState:debugEnabled ? NSControlStateValueOn : NSControlStateValueOff];
}

- (void)exitApp:(id)sender {
    printf("Exit app called\n");
    shouldKeepRunning = false;
    cleanup_and_exit();
}

@end

@interface UIThread : NSThread
@end

@implementation UIThread
- (void)main {
    @autoreleasepool {
        printf("Starting UI thread\n");
        [[NSThread currentThread] setName:@"UIThread"];
        
        NSApplication *app = [NSApplication sharedApplication];
        printf("NSApplication created\n");
        
        delegate = [[StatusBarDelegate alloc] init];
        printf("Delegate created\n");
        
        [app setDelegate:delegate];
        printf("Delegate set\n");
        
        // Prevent the app from appearing in the dock
        [app setActivationPolicy:NSApplicationActivationPolicyAccessory];
        printf("Activation policy set\n");
        
        [app finishLaunching];
        printf("App finished launching\n");
        
        // Create a timer to check if we should keep running
        NSTimer *timer = [NSTimer scheduledTimerWithTimeInterval:0.1
                                                       repeats:YES
                                                         block:^(NSTimer *timer) {
            if (!shouldKeepRunning) {
                [timer invalidate];
                [NSApp stop:nil];
            }
        }];
        [[NSRunLoop currentRunLoop] addTimer:timer forMode:NSRunLoopCommonModes];
        
        printf("Starting run loop\n");
        [[NSRunLoop currentRunLoop] run];
        printf("Run loop ended\n");
    }
}
@end

void create_status_bar() {
    printf("Creating status bar\n");
    if (!statusItem) {
        statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
        if (!statusItem) {
            printf("Failed to create status item!\n");
            return;
        }
        printf("Status item created\n");
        [statusItem retain];
        
        // Create the menu
        statusMenu = [[NSMenu alloc] init];
        if (!statusMenu) {
            printf("Failed to create menu!\n");
            return;
        }
        printf("Menu created\n");
        [statusMenu setAutoenablesItems:NO];
        
        // Create menu items
        NSMenuItem *debugItem = [[NSMenuItem alloc] initWithTitle:@"Debug Mode" 
                                                         action:@selector(toggleDebug:) 
                                                  keyEquivalent:@""];
        if (!debugItem) {
            printf("Failed to create debug menu item!\n");
            return;
        }
        printf("Debug menu item created\n");
        
        [debugItem setTarget:delegate];
        [debugItem setTag:1];
        [debugItem setState:debugEnabled ? NSControlStateValueOn : NSControlStateValueOff];
        
        NSMenuItem *exitItem = [[NSMenuItem alloc] initWithTitle:@"Exit" 
                                                        action:@selector(exitApp:) 
                                                 keyEquivalent:@"q"];
        if (!exitItem) {
            printf("Failed to create exit menu item!\n");
            return;
        }
        printf("Exit menu item created\n");
        [exitItem setTarget:delegate];
        
        // Add items to menu
        [statusMenu addItem:debugItem];
        [statusMenu addItem:[NSMenuItem separatorItem]];
        [statusMenu addItem:exitItem];
        printf("Menu items added\n");
        
        [debugItem release];
        [exitItem release];
        
        // Set the menu
        [statusItem setMenu:statusMenu];
        printf("Menu set to status item\n");
        
        // Set initial title
        if ([statusItem button]) {
            [[statusItem button] setTitle:@"I"];
            printf("Initial title set to I\n");
        } else {
            printf("Warning: status item button is nil!\n");
        }
    } else {
        printf("Status item already exists\n");
    }
}

void setup_status_bar() {
    printf("Setting up status bar\n");
    
    // Create and start the UI thread
    uiThread = [[UIThread alloc] init];
    [uiThread start];
    printf("UI thread started\n");
}

void update_status_bar_icon(char mode) {
    printf("Updating status bar icon to: %c\n", mode);
    dispatch_async(dispatch_get_main_queue(), ^{
        if (!statusItem) {
            printf("Status item is nil during update!\n");
            return;
        }
        
        // Update the status bar text with the current mode
        NSString *modeText = [NSString stringWithFormat:@"%c", mode];
        if ([statusItem button]) {
            [[statusItem button] setTitle:modeText];
            printf("Status bar title updated to: %c\n", mode);
        } else {
            printf("Warning: status item button is nil during update!\n");
        }
    });
}

void cleanup_status_bar() {
    printf("Cleaning up status bar\n");
    shouldKeepRunning = false;
    
    dispatch_sync(dispatch_get_main_queue(), ^{
        if (statusItem) {
            [[NSStatusBar systemStatusBar] removeStatusItem:statusItem];
            [statusItem release];
            statusItem = nil;
            printf("Status item cleaned up\n");
        }
        
        if (statusMenu) {
            [statusMenu release];
            statusMenu = nil;
            printf("Menu cleaned up\n");
        }
        
        if (delegate) {
            [delegate release];
            delegate = nil;
            printf("Delegate cleaned up\n");
        }
        
        printf("Terminating NSApp\n");
        [NSApp terminate:nil];
    });
    
    // Wait for the UI thread to finish
    [uiThread cancel];
    [uiThread release];
    uiThread = nil;
} 