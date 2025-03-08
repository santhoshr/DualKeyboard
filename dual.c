// dual.c
// http://osxbook.com
// modified by Chance Miller to support multikeyboard use
//
// Complile using the following command line:
//	 gcc -Wall -o dual dual.c -framework ApplicationServices
//
// You need superuser privileges to create the event tap, unless accessibility
// is enabled. To do so, select the "Enable access for assistive devices"
// checkbox in the Universal Access system preference pane.

#include <ApplicationServices/ApplicationServices.h>
#include <time.h>  // For measuring time durations
#include <stdbool.h> // For boolean types
#include <stdio.h>  // For printf
#include <stdlib.h> // For system() and atexit()
#include <string.h> // For strcmp
#include <unistd.h> // For access() and getpid()
#include <signal.h> // For signal handling
#include <fcntl.h>  // For file operations
#include <sys/file.h> // For flock()

// Debug flag
bool debug_mode = false;

// Lock file path for single instance check
#define LOCK_FILE "/tmp/dual.lock"
int lock_fd = -1;

// Define keycodes
#define KEYCODE_SECTION 10      // § symbol - CapsLock gets remapped to this
#define KEYCODE_ESCAPE 53
#define KEYCODE_LEFT_ARROW 123
#define KEYCODE_RIGHT_ARROW 124
#define KEYCODE_DOWN_ARROW 125
#define KEYCODE_UP_ARROW 126
#define KEYCODE_PAGE_UP 116
#define KEYCODE_PAGE_DOWN 121
#define KEYCODE_HOME 115
#define KEYCODE_END 119
#define KEYCODE_H 4
#define KEYCODE_J 38
#define KEYCODE_K 40
#define KEYCODE_L 37
#define KEYCODE_I 34
#define KEYCODE_O 31
#define KEYCODE_COMMA 43
#define KEYCODE_PERIOD 47
#define KEYCODE_SPACE 49

//Global Variables to keep track of modifier keys pressed
bool ctr = false;
bool sft = false;
bool cmd = false;
bool opt = false;

// Variables for CapsLock dual functionality 
bool caps_key_down = false;
uint64_t caps_press_time = 0;
bool vim_mode_active = false;
bool key_repeat = false;  // Flag to track key repeat
const uint64_t HOLD_THRESHOLD = 200000000; // 200ms in nanoseconds

// Variables for exit key combination
bool escape_pressed = false;
bool control_pressed = false;
bool space_pressed = false;

// Forward declarations
void restore_capslock_mapping();
uint64_t get_current_time_ns();
void send_escape_key();

// Function to clean up resources before exit
void cleanup_and_exit() {
	// Restore original keyboard mapping
	restore_capslock_mapping();
	
	// Release the lock file
	if (lock_fd != -1) {
		flock(lock_fd, LOCK_UN);
		close(lock_fd);
		unlink(LOCK_FILE);
	}
	
	if (debug_mode) {
		printf("Exiting dual program...\n");
	}
	
	exit(0);
}

// Signal handler for graceful termination
void signal_handler(int signum) {
	if (debug_mode) {
		printf("Received signal %d, exiting...\n", signum);
	}
	cleanup_and_exit();
}

// Function to check if another instance is running
bool is_another_instance_running() {
	// Create or open the lock file
	lock_fd = open(LOCK_FILE, O_CREAT | O_RDWR, 0666);
	if (lock_fd == -1) {
		perror("Failed to open lock file");
		return false; // Assume no other instance is running if we can't check
	}
	
	// Try to get an exclusive lock
	if (flock(lock_fd, LOCK_EX | LOCK_NB) == -1) {
		// Another instance has the lock
		close(lock_fd);
		lock_fd = -1;
		return true;
	}
	
	// Write PID to lock file
	char pid_str[16];
	sprintf(pid_str, "%d\n", getpid());
	ftruncate(lock_fd, 0);
	write(lock_fd, pid_str, strlen(pid_str));
	
	// We got the lock, no other instance is running
	return false;
}

// Function to print keycode information in debug mode
void debug_print_key(CGKeyCode keycode, CGEventType type) {
	if (!debug_mode) return;
	
	const char* event_type = "";
	if (type == kCGEventKeyDown) event_type = "KeyDown";
	else if (type == kCGEventKeyUp) event_type = "KeyUp";
	else if (type == kCGEventFlagsChanged) event_type = "FlagsChanged";
	
	char* key_name = "";
	switch (keycode) {
		case KEYCODE_SECTION: key_name = "Section § (10) [CapsLock remapped]"; break;
		case KEYCODE_ESCAPE: key_name = "Escape (53)"; break;
		case KEYCODE_LEFT_ARROW: key_name = "LeftArrow (123)"; break;
		case KEYCODE_RIGHT_ARROW: key_name = "RightArrow (124)"; break;
		case KEYCODE_DOWN_ARROW: key_name = "DownArrow (125)"; break;
		case KEYCODE_UP_ARROW: key_name = "UpArrow (126)"; break;
		case KEYCODE_PAGE_UP: key_name = "PageUp (116)"; break;
		case KEYCODE_PAGE_DOWN: key_name = "PageDown (121)"; break;
		case KEYCODE_HOME: key_name = "Home (115)"; break;
		case KEYCODE_END: key_name = "End (119)"; break;
		case KEYCODE_H: key_name = "H (4)"; break;
		case KEYCODE_J: key_name = "J (38)"; break;
		case KEYCODE_K: key_name = "K (40)"; break;
		case KEYCODE_L: key_name = "L (37)"; break;
		case KEYCODE_I: key_name = "I (34)"; break;
		case KEYCODE_O: key_name = "O (31)"; break;
		case KEYCODE_COMMA: key_name = "Comma (43)"; break;
		case KEYCODE_PERIOD: key_name = "Period (47)"; break;
		default: key_name = ""; break;
	}
	
	printf("Event: %s, KeyCode: %d %s\n", event_type, (int)keycode, key_name);
}

// Get current time in nanoseconds
uint64_t get_current_time_ns() {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

// Function to set up the CapsLock remapping
void setup_capslock_remapping() {
	// Remap CapsLock (0x700000039) to Section key which is keycode 10
	// This is what appears to be happening on this particular Mac
	const char* remap_cmd = "hidutil property --set '{\"UserKeyMapping\":[{\"HIDKeyboardModifierMappingSrc\":0x700000039,\"HIDKeyboardModifierMappingDst\":0x700000064}]}'";
	
	// Execute the command and capture its output
	FILE* pipe = popen(remap_cmd, "r");
	if (!pipe) {
		fprintf(stderr, "Failed to execute hidutil command.\n");
		return;
	}
	
	// Read the output
	char buffer[512];
	while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
		if (debug_mode) {
			printf("%s", buffer);
		}
	}
	
	int result = pclose(pipe);
	
	if (result != 0) {
		fprintf(stderr, "Failed to remap CapsLock. You may need to run this program with sudo.\n");
	} else if (debug_mode) {
		printf("CapsLock remapping applied successfully.\n");
		printf("Now watching for keycode 10 (§ Section key) events from CapsLock.\n");
	}
}

// Function to restore the original CapsLock mapping when program exits
void restore_capslock_mapping() {
	// Clear the UserKeyMapping to restore default behavior
	const char* restore_cmd = "hidutil property --set '{\"UserKeyMapping\":[]}'";
	
	// Execute the command and capture its output
	FILE* pipe = popen(restore_cmd, "r");
	if (!pipe) {
		fprintf(stderr, "Failed to execute hidutil command.\n");
		return;
	}
	
	// Read the output
	char buffer[512];
	while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
		if (debug_mode) {
			printf("%s", buffer);
		}
	}
	
	pclose(pipe);
	
	if (debug_mode) {
		printf("Original keyboard mapping restored.\n");
	}
}

// Function to send an Escape key
void send_escape_key() {
	if (debug_mode) {
		printf("Sending Escape key\n");
	}
	
	// Create a keyboard event source
	CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
	if (!source) {
		fprintf(stderr, "Failed to create event source\n");
		return;
	}
	
	// Create and post the escape key events with the source
	CGEventRef escape_down = CGEventCreateKeyboardEvent(source, KEYCODE_ESCAPE, true);
	CGEventRef escape_up = CGEventCreateKeyboardEvent(source, KEYCODE_ESCAPE, false);
	
	// Post events with a small delay to ensure they're processed
	CGEventPost(kCGHIDEventTap, escape_down);
	usleep(1000); // 1ms delay
	CGEventPost(kCGHIDEventTap, escape_up);
	
	// Clean up
	CFRelease(escape_down);
	CFRelease(escape_up);
	CFRelease(source);
}

// This callback will be invoked every time there is a keystroke.
CGEventRef
myCGEventCallback(CGEventTapProxy proxy, CGEventType type,
				  CGEventRef event, void *refcon)
{
	// Paranoid sanity check.
	if ((type != kCGEventKeyDown) && (type != kCGEventKeyUp) && (type != kCGEventFlagsChanged))
		return event;

	// The incoming keycode.
	CGKeyCode keycode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
	
	// Print debug info if debug mode is on
	debug_print_key(keycode, type);
	
	// Track keys for exit combination (Escape + Control + Space)
	if (keycode == KEYCODE_ESCAPE) {
		escape_pressed = (type == kCGEventKeyDown);
	} else if (keycode == (CGKeyCode)59 || keycode == (CGKeyCode)62) {
		// Control key
		control_pressed = (type == kCGEventKeyDown || type == kCGEventFlagsChanged);
	} else if (keycode == KEYCODE_SPACE) {
		space_pressed = (type == kCGEventKeyDown);
	}
	
	// Check for exit combination
	if (escape_pressed && control_pressed && space_pressed) {
		if (debug_mode) {
			printf("Exit key combination detected (Escape + Control + Space)\n");
		}
		cleanup_and_exit();
	}
	
	// Handle remapped CapsLock key (now Section key, keycode 10)
	if (keycode == KEYCODE_SECTION) {
		if (type == kCGEventKeyDown) {
			// Check if this is a repeat or new press
			uint64_t current_time = get_current_time_ns();
			CGEventFlags flags = CGEventGetFlags(event);
			bool is_repeat = (flags & kCGEventFlagMaskNonCoalesced) == 0;
			
			if (!caps_key_down) {
				// Initial key press
				caps_key_down = true;
				caps_press_time = current_time;
				vim_mode_active = false;
				key_repeat = false;
				
				if (debug_mode) {
					printf("CapsLock (§ key) pressed down - INITIAL PRESS\n");
				}
				
				// Suppress the original event
				return NULL;
			} else if (is_repeat) {
				// This is a key repeat, mark it as such
				key_repeat = true;
				
				if (debug_mode) {
					printf("CapsLock (§ key) key repeat detected - ignoring\n");
				}
				
				// Suppress repeats
				return NULL;
			}
		}
		else if (type == kCGEventKeyUp && caps_key_down) {
			// Key was released
			uint64_t release_time = get_current_time_ns();
			uint64_t hold_duration = release_time - caps_press_time;
			
			caps_key_down = false;
			key_repeat = false;
			
			// Only send Escape if key was tapped briefly and Vim mode wasn't activated
			if (!vim_mode_active && hold_duration < HOLD_THRESHOLD) {
				send_escape_key();
			}
			
			// End Vim navigation mode
			vim_mode_active = false;
			
			// Suppress the original event
			return NULL;
		}
		
		// Always suppress any other events related to the CapsLock key
		// This ensures no special characters are emitted when holding CapsLock
		return NULL;
	}
	
	// Handle Vim navigation when CapsLock is held down
	if (caps_key_down && type == kCGEventKeyDown) {
		// Create a new event source for better event handling
		CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
		if (!source) {
			return event;
		}
		
		uint64_t current_time = get_current_time_ns();
		uint64_t hold_duration = current_time - caps_press_time;
		
		// Only enable Vim navigation if CapsLock has been held long enough
		if (hold_duration >= HOLD_THRESHOLD) {
			// Mark that we're in Vim mode (which prevents Escape on release)
			if (!vim_mode_active) {
				vim_mode_active = true;
				if (debug_mode) {
					printf("Vim navigation mode activated\n");
				}
			}
			
			CGKeyCode new_keycode = 0;
			bool should_remap = true;
			
			// Map keys to vim navigation
			switch (keycode) {
				case KEYCODE_H: new_keycode = KEYCODE_LEFT_ARROW; break;
				case KEYCODE_J: new_keycode = KEYCODE_DOWN_ARROW; break;
				case KEYCODE_K: new_keycode = KEYCODE_UP_ARROW; break;
				case KEYCODE_L: new_keycode = KEYCODE_RIGHT_ARROW; break;
				case KEYCODE_I: new_keycode = KEYCODE_PAGE_UP; break;
				case KEYCODE_O: new_keycode = KEYCODE_PAGE_DOWN; break;
				case KEYCODE_COMMA: new_keycode = KEYCODE_HOME; break;
				case KEYCODE_PERIOD: new_keycode = KEYCODE_END; break;
				default: should_remap = false; break;
			}
			
			if (should_remap) {
				// Create a new event with the remapped keycode
				CGEventRef new_event = CGEventCreateKeyboardEvent(source, new_keycode, true);
				CGEventSetFlags(new_event, CGEventGetFlags(event));
				
				// Post the event directly to the HID system
				CGEventPost(kCGHIDEventTap, new_event);
				CFRelease(new_event);
				CFRelease(source);
				return NULL;
			}
			
			CFRelease(source);
		}
	}
	// Handle key up events for vim navigation keys
	else if (caps_key_down && type == kCGEventKeyUp && vim_mode_active) {
		// Create a new event source for better event handling
		CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
		if (!source) {
			return event;
		}
		
		CGKeyCode new_keycode = 0;
		bool should_remap = true;
		
		// Map keys to vim navigation (same mapping as for key down)
		switch (keycode) {
			case KEYCODE_H: new_keycode = KEYCODE_LEFT_ARROW; break;
			case KEYCODE_J: new_keycode = KEYCODE_DOWN_ARROW; break;
			case KEYCODE_K: new_keycode = KEYCODE_UP_ARROW; break;
			case KEYCODE_L: new_keycode = KEYCODE_RIGHT_ARROW; break;
			case KEYCODE_I: new_keycode = KEYCODE_PAGE_UP; break;
			case KEYCODE_O: new_keycode = KEYCODE_PAGE_DOWN; break;
			case KEYCODE_COMMA: new_keycode = KEYCODE_HOME; break;
			case KEYCODE_PERIOD: new_keycode = KEYCODE_END; break;
			default: should_remap = false; break;
		}
		
		if (should_remap) {
			if (debug_mode) {
				printf("Sending key up event for remapped key: %d\n", (int)new_keycode);
			}
			
			// Create a new event with the remapped keycode (key up)
			CGEventRef new_event = CGEventCreateKeyboardEvent(source, new_keycode, false);
			CGEventSetFlags(new_event, CGEventGetFlags(event));
			
			// Post the event directly to the HID system
			CGEventPost(kCGHIDEventTap, new_event);
			CFRelease(new_event);
			CFRelease(source);
			return NULL;
		}
		
		CFRelease(source);
	}
	
	//Control
	if(keycode == (CGKeyCode)59||keycode == (CGKeyCode)62){
		ctr = !ctr;
	}
	if(ctr){
		CGEventSetFlags(event,NX_CONTROLMASK|CGEventGetFlags(event));
	}
	//Shift
	if(keycode == (CGKeyCode)60||keycode == (CGKeyCode)56){
		sft = !sft;
	}
	if(sft){
		CGEventSetFlags(event,NX_SHIFTMASK|CGEventGetFlags(event));
	}
	//Command
	if(keycode == (CGKeyCode)55||keycode == (CGKeyCode)54){
		cmd = !cmd;
	}
	if(cmd){
		CGEventSetFlags(event,NX_COMMANDMASK|CGEventGetFlags(event));
	}
	//Option
	if(keycode == (CGKeyCode)58||keycode == (CGKeyCode)61){
		opt = !opt;
	}
	if(opt){
		CGEventSetFlags(event,NX_ALTERNATEMASK|CGEventGetFlags(event));
	}
	CGEventSetIntegerValueField(
		event, kCGKeyboardEventKeycode, (int64_t)keycode);

	// We must return the event for it to be useful.
	return event;
}

int
main(int argc, char* argv[])
{
	// Parse command line arguments
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-debug") == 0) {
			debug_mode = true;
			printf("Debug mode enabled. Key events will be printed.\n");
		}
	}
	
	// Check if another instance is already running
	if (is_another_instance_running()) {
		fprintf(stderr, "Another instance of dual is already running.\n");
		fprintf(stderr, "Please exit the existing instance before starting a new one.\n");
		return 1;
	}
	
	// Set up signal handlers for graceful termination
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGHUP, signal_handler);
	
	// Set up CapsLock remapping
	setup_capslock_remapping();
	
	// Register the cleanup function to be called on program exit
	atexit(restore_capslock_mapping);

	CFMachPortRef	  eventTap;
	CGEventMask		eventMask;
	CFRunLoopSourceRef runLoopSource;

	// Create an event tap. We are interested in key presses.
	eventMask = ((1 << kCGEventKeyDown) | (1 << kCGEventKeyUp) | (1 << kCGEventFlagsChanged));
	eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, 0,
								eventMask, myCGEventCallback, NULL);
	if (!eventTap) {
		fprintf(stderr, "failed to create event tap\n");
		cleanup_and_exit();
		return 1;
	}

	// Create a run loop source.
	runLoopSource = CFMachPortCreateRunLoopSource(
						kCFAllocatorDefault, eventTap, 0);

	// Add to the current run loop.
	CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource,
					   kCFRunLoopCommonModes);

	// Enable the event tap.
	CGEventTapEnable(eventTap, true);
	
	if (debug_mode) {
		printf("dual is running. Press Escape + Control + Space to exit.\n");
	}

	// Set it all running.
	CFRunLoopRun();

	// Cleanup before exit
	cleanup_and_exit();
	return 0;
}

