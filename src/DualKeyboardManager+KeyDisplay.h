#import "DualKeyboard.h"

@interface DualKeyboardManager (KeyDisplay)
- (void)toggleKeyDisplay;
- (void)updateKeyDisplay:(CGKeyCode)keycode flags:(CGEventFlags)flags isKeyDown:(BOOL)isDown;
- (void)refreshKeyDisplayModifiers;
@end
