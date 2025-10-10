#ifndef POSIX_SUPPORT_H
#define POSIX_SUPPORT_H

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

void setTerminalRawMode();
void resetTerminalMode();
int _getch();
int _kbhit(void);

#endif // POSIX_SUPPORT_H