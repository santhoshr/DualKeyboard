#include "../include/capslock.h"

// Variables for CapsLock dual functionality 
bool caps_key_down = false;
uint64_t caps_press_time = 0;
bool key_repeat = false;  // Flag to track key repeat
const uint64_t HOLD_THRESHOLD = 200000000; // 200ms in nanoseconds

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