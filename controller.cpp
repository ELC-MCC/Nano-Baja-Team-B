#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <linux/joystick.h>

// Configuration - Adjust these to match your setup
#define SERIAL_PORT "/dev/ttyS0" // Or "/dev/ttyUSB0" if using USB to serial
#define JOYSTICK_DEV "/dev/input/js0"
#define BAUD_RATE B57600

int openSerial(const char* port) {
    int fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("Unable to open serial port");
        return -1;
    }

    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, BAUD_RATE);
    cfsetospeed(&options, BAUD_RATE);
    
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB; // No parity
    options.c_cflag &= ~CSTOPB; // 1 stop bit
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;     // 8 data bits
    
    tcsetattr(fd, TCSANOW, &options);
    return fd;
}

int main() {
    int serialFd = openSerial(SERIAL_PORT);
    int joyFd = open(JOYSTICK_DEV, O_RDONLY);

    if (serialFd == -1 || joyFd == -1) {
        return 1;
    }

    std::cout << "Controller connected. Ready to drive!" << std::endl;

    struct js_event event;
    char lastCommand = ' ';
    char currentCommand = 'S'; // Default to Stop

    while (true) {
        // Read joystick events
        if (read(joyFd, &event, sizeof(event)) > 0) {
            
            // Handle Axis movements (Left Stick or D-Pad)
            if (event.type == JS_EVENT_AXIS) {
                // Y-Axis (Usually Axis 1): Forward/Back
                if (event.number == 1) {
                    if (event.value < -16000) currentCommand = 'W';      // Up
                    else if (event.value > 16000) currentCommand = 'S'; // Down
                    else currentCommand = 'X';                          // Center/Stop
                }
                // X-Axis (Usually Axis 0): Left/Right
                else if (event.number == 0) {
                    if (event.value < -16000) currentCommand = 'A';      // Left
                    else if (event.value > 16000) currentCommand = 'D'; // Right
                }
            }

            // Only send if the command has changed to avoid flooding the modem
            if (currentCommand != lastCommand) {
                write(serialFd, &currentCommand, 1);
                std::cout << "Sent Command: " << currentCommand << std::endl;
                lastCommand = currentCommand;
            }
        }
        
        // Small sleep to keep CPU usage low
        usleep(10000); 
    }

    close(serialFd);
    close(joyFd);
    return 0;
}