#import "Dual.h"

@interface DualManager (SingleInstance)

- (BOOL)ensureSingleInstance;
- (void)cleanupSingleInstance;

@end