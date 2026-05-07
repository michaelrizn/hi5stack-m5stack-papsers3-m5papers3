#ifndef DEBUG_CONFIG_H
#define DEBUG_CONFIG_H


// Enable only the debug channels needed for the current investigation.
// #define DEBUG_TOUCH
// #define DEBUG_WIFI_TOUCH
// #define DEBUG_ALL

#ifdef DEBUG_ALL
    #define DEBUG_TOUCH
    #define DEBUG_KEYBOARD
    #define DEBUG_FILES
    #define DEBUG_SD_GATEWAY
#endif

#endif
