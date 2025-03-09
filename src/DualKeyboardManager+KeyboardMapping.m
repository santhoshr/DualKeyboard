#import "DualKeyboardManager+KeyboardMapping.h"
#import "DualKeyboardManager+CapsNavigation.h"
#import "DualKeyboardManager+MenuBar.h"
#import <mach/mach_time.h>

// Key tracking for shortcuts
static BOOL escapePressed = NO;
static BOOL controlPressed = NO;
static BOOL spacePressed = NO;
static BOOL zeroPressed = NO;
static BOOL minusPressed = NO;

// Modifier key states
static BOOL leftShiftDown = NO;
static BOOL rightShiftDown = NO;
static BOOL leftControlDown = NO;
static BOOL rightControlDown = NO;
static BOOL leftCommandDown = NO;
static BOOL rightCommandDown = NO;
static BOOL leftOptionDown = NO;
static BOOL rightOptionDown = NO;

@implementation DualKeyboardManager (KeyboardMapping)

- (void)restartApplication {
    NSString *executablePath = [[NSBundle mainBundle] executablePath];
    NSTask *task = [[NSTask alloc] init];
    task.launchPath = executablePath;
    
    NSMutableArray *args = [NSMutableArray array];
    if (self.debugMode) [args addObject:@"-d"];
    task.arguments = args;
    
    [task launch];
}

- (BOOL)handleKeyEvent:(CGEventRef)event ofType:(CGEventType)type withKeycode:(CGKeyCode)keycode {
    CGEventFlags flags = CGEventGetFlags(event);
    
    // Handle caps navigation first
    if ([self handleCapsNavigation:event ofType:type withKeycode:keycode flags:flags]) {
        return YES;
    }
    
    // Handle modifier keys
    if (type == kCGEventFlagsChanged) {
        // Track individual modifier keys
        switch (keycode) {
            case 56:  // Left shift
                leftShiftDown = (flags & kCGEventFlagMaskShift) != 0;
                break;
            case 60:  // Right shift
                rightShiftDown = (flags & kCGEventFlagMaskShift) != 0;
                break;
            case 59:  // Left control
                leftControlDown = (flags & kCGEventFlagMaskControl) != 0;
                break;
            case 62:  // Right control
                rightControlDown = (flags & kCGEventFlagMaskControl) != 0;
                break;
            case 58:  // Left option
                leftOptionDown = (flags & kCGEventFlagMaskAlternate) != 0;
                break;
            case 61:  // Right option
                rightOptionDown = (flags & kCGEventFlagMaskAlternate) != 0;
                break;
            case 55:  // Left command
                leftCommandDown = (flags & kCGEventFlagMaskCommand) != 0;
                break;
            case 54:  // Right command
                rightCommandDown = (flags & kCGEventFlagMaskCommand) != 0;
                break;
        }
        
        // Build combined modifier flags
        CGEventFlags newFlags = 0;
        if (leftShiftDown || rightShiftDown) newFlags |= kCGEventFlagMaskShift;
        if (leftControlDown || rightControlDown) newFlags |= kCGEventFlagMaskControl;
        if (leftOptionDown || rightOptionDown) newFlags |= kCGEventFlagMaskAlternate;
        if (leftCommandDown || rightCommandDown) newFlags |= kCGEventFlagMaskCommand;
        
        // Update control pressed state for shortcuts
        controlPressed = leftControlDown || rightControlDown;
        
        if (self.debugMode) {
            NSLog(@"Modifier update - shift:%d ctrl:%d opt:%d cmd:%d", 
                  (leftShiftDown || rightShiftDown),
                  (leftControlDown || rightControlDown),
                  (leftOptionDown || rightOptionDown),
                  (leftCommandDown || rightCommandDown));
        }
        
        CGEventSetFlags(event, newFlags);
        return NO;
    }
    
    // For normal key events, apply combined modifier states
    if (type == kCGEventKeyDown || type == kCGEventKeyUp) {
        CGEventFlags newFlags = 0;
        if (leftShiftDown || rightShiftDown) newFlags |= kCGEventFlagMaskShift;
        if (leftControlDown || rightControlDown) newFlags |= kCGEventFlagMaskControl;
        if (leftOptionDown || rightOptionDown) newFlags |= kCGEventFlagMaskAlternate;
        if (leftCommandDown || rightCommandDown) newFlags |= kCGEventFlagMaskCommand;
        
        CGEventSetFlags(event, newFlags);
    }

    // Handle Escape key
    if (keycode == 53) {
        if (type == kCGEventKeyDown && !escapePressed) {
            escapePressed = YES;
        } else if (type == kCGEventKeyUp) {
            escapePressed = NO;
        }
    }

    // Handle Control key for shortcuts
    if (keycode == 59 || keycode == 62) {
        if (type == kCGEventKeyDown) {
            controlPressed = YES;
        } else if (type == kCGEventKeyUp) {
            controlPressed = NO;
        }
    }

    // Handle Space key
    if (keycode == 49) {
        if (type == kCGEventKeyDown) {
            spacePressed = YES;
        } else if (type == kCGEventKeyUp) {
            spacePressed = NO;
        }
    }

    // Handle Zero key for restart shortcut
    if (keycode == 29) {
        if (type == kCGEventKeyDown) {
            zeroPressed = YES;
            if (escapePressed) {
                if (self.debugMode) {
                    NSLog(@"Restart key combination detected");
                }
                [self cleanup];
                [self restartApplication];
                exit(0);
            }
        } else if (type == kCGEventKeyUp) {
            zeroPressed = NO;
        }
    }

    // Handle Minus key for debug toggle
    if (keycode == 27) {
        if (type == kCGEventKeyDown) {
            minusPressed = YES;
            if (escapePressed && self.debugModeAtStartup) {  // Check if debug was enabled at startup
                self.debugMode = !self.debugMode;
                [self updateMenuBarStatus];
                printf("\nDebug messages %s\n", self.debugMode ? "enabled" : "disabled");
                return YES;
            }
        } else if (type == kCGEventKeyUp) {
            minusPressed = NO;
        }
    }

    // Check for exit combination
    if (escapePressed && controlPressed && spacePressed) {
        if (self.debugMode) {
            NSLog(@"Exit key combination detected");
        }
        [self cleanup];
        exit(0);
    }

    return NO;
}

@end