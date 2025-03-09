#include "../include/termstatus.h"

// Current mode
static char current_mode = MODE_INSERT;

void setup_status_bar() {
    if (quiet_mode) return;
    
    printf("\n\033[1;36m╔════════════════════════════╗\033[0m\n");
    printf("\033[1;36m║     Dual Keyboard Mode     ║\033[0m\n");
    printf("\033[1;36m╠════════════════════════════╣\033[0m\n");
    printf("\033[1;36m║\033[0m Current Mode: \033[1;32m%c\033[0m          \033[1;36m║\033[0m\n", current_mode);
    printf("\033[1;36m║\033[0m Debug Mode: \033[1;33m%s\033[0m        \033[1;36m║\033[0m\n", debug_mode ? "ON" : "OFF");
    printf("\033[1;36m╚════════════════════════════╝\033[0m\n\n");
    fflush(stdout);
}

void update_status_bar_icon(char mode) {
    if (mode != MODE_INSERT && mode != MODE_NAVIGATION) {
        return;
    }

    // Only update if the mode has changed
    if (mode == current_mode) {
        return;
    }

    current_mode = mode;
    
    if (!quiet_mode) {
        printf("\n\033[1;33m╔════════════════════════════╗\033[0m\n");
        printf("\033[1;33m║       Mode Changed!        ║\033[0m\n");
        printf("\033[1;33m╠════════════════════════════╣\033[0m\n");
        printf("\033[1;33m║\033[0m New Mode: \033[1;32m%c\033[0m             \033[1;33m║\033[0m\n", current_mode);
        printf("\033[1;33m║\033[0m Debug Mode: \033[1;33m%s\033[0m        \033[1;33m║\033[0m\n", debug_mode ? "ON" : "OFF");
        printf("\033[1;33m╚════════════════════════════╝\033[0m\n\n");
        fflush(stdout);
    }
}

void cleanup_status_bar() {
    if (quiet_mode) return;
    
    printf("\n\033[1;31m╔════════════════════════════╗\033[0m\n");
    printf("\033[1;31m║    Dual Keyboard Exit     ║\033[0m\n");
    printf("\033[1;31m╠════════════════════════════╣\033[0m\n");
    printf("\033[1;31m║\033[0m Final Mode: \033[1;32m%c\033[0m           \033[1;31m║\033[0m\n", current_mode);
    printf("\033[1;31m║\033[0m Debug Mode: \033[1;33m%s\033[0m        \033[1;31m║\033[0m\n", debug_mode ? "ON" : "OFF");
    printf("\033[1;31m╚════════════════════════════╝\033[0m\n\n");
    fflush(stdout);
} 