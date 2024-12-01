#include <windows.h>

// Define the usleep function for Windows
void win_compat_usleep(unsigned int microseconds) {
    // Convert microseconds to milliseconds
    unsigned int milliseconds = microseconds / 1000;
    Sleep(milliseconds);
}
