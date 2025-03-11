#import "Dual.h"

@interface DualManager (ConsoleWindow)

- (void)createConsoleWindowIfNeeded;
- (void)closeConsoleWindow;
- (void)appendToConsole:(NSString *)text;

@end
