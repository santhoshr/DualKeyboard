#ifndef CAPSLOCK_H
#define CAPSLOCK_H

#include "common.h"

// CapsLock functionality
void setup_capslock_remapping();
void restore_capslock_mapping();
void send_escape_key();

// Global variables for CapsLock functionality
extern bool caps_key_down;
extern uint64_t caps_press_time;
extern bool key_repeat;
extern const uint64_t HOLD_THRESHOLD;

#endif // CAPSLOCK_H 