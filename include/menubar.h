#ifndef MENUBAR_H
#define MENUBAR_H

#include "common.h"

// Menu bar functionality
void setup_menu_bar();
void update_menu_bar_icon(char mode);
void cleanup_menu_bar();

// Status bar functionality
void setup_status_bar();
void create_status_bar();
void update_status_bar_icon(char mode);
void cleanup_status_bar();

// Menu bar mode constants
#define MODE_INSERT 'I'
#define MODE_NAVIGATION 'N'

#endif // MENUBAR_H 