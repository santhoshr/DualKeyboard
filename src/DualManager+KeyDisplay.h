#import "Dual.h"

@interface DualManager (KeyDisplay)
- (void)toggleKeyDisplay;
- (void)updateKeyDisplay:(CGKeyCode)keycode flags:(CGEventFlags)flags isKeyDown:(BOOL)isDown;
- (void)refreshKeyDisplayModifiers;
@end
