#import "Dual.h"
#import "DualManager+KeyboardStatus.h"

// Expose modifier key states
extern BOOL leftShiftDown;
extern BOOL rightShiftDown;
extern BOOL leftControlDown;
extern BOOL rightControlDown;
extern BOOL leftCommandDown;
extern BOOL rightCommandDown;
extern BOOL leftOptionDown;
extern BOOL rightOptionDown;

@interface DualManager (KeyboardMapping)
- (BOOL)handleKeyEvent:(CGEventRef)event ofType:(CGEventType)type withKeycode:(CGKeyCode)keycode;
- (void)restartApplication;
- (CGEventFlags)computeCombinedModifierFlags;
- (void)notifyModifierChanges:(CGKeyCode)keycode flags:(CGEventFlags)flags modifierChanged:(BOOL)modifierChanged;
@end