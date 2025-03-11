#import "Dual.h"

@interface DualManager (KeyboardStatus)
- (void)setupStatusBar;
- (void)updateStatusWithMode:(char)mode;
- (void)cleanupStatusBar;
@end