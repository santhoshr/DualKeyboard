#import "DualKeyboard.h"

@interface DualKeyboardManager (ConsoleWindow)

- (void)createConsoleWindowIfNeeded;
- (void)closeConsoleWindow;
- (void)appendToConsole:(NSString *)text;

@end
