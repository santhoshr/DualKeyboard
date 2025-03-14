// Dual.h
#import <Foundation/Foundation.h>
#import <Carbon/Carbon.h>
#import <AppKit/AppKit.h>

@interface DualManager : NSObject

@property (nonatomic, assign) BOOL debugMode;
@property (nonatomic, assign) BOOL debugModeAtStartup;  // Track if debug was enabled at launch
@property (nonatomic, assign) BOOL shouldRestart;
@property (nonatomic, assign) char currentMode;
@property (nonatomic, strong) NSStatusItem *statusItem;
@property (nonatomic, strong) NSMenu *statusMenu;
@property (nonatomic, strong) NSWindow *consoleWindow;  // Add this line

+ (instancetype)sharedInstance;
- (BOOL)startEventTap;
- (void)cleanup;

@end