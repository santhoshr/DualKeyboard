#ifndef TERMSTATUS_H
#define TERMSTATUS_H

#include "common.h"

// Status bar functionality
void setup_status_bar();
void update_status_msg(char mode);
void cleanup_status_bar();

// Status bar mode constants
#define MODE_INSERT 'I'
#define MODE_NAVIGATION 'N'

#endif // TERMSTATUS_H 