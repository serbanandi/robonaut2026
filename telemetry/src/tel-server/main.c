#include <assert.h>
#include <fcntl.h>
#include <log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timerfd.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <zmq.h>
#include "ctrl/ctrl_interface.h"

#define ZMQ_PUB_ADDR "tcp://*:5555"
#define ZMQ_REP_ADDR "tcp://*:5556"

int getTermiosBadrate(int baudrate)
{
    switch (baudrate)
    {
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        case 230400: return B230400;
        case 460800: return B460800;
        case 921600: return B921600;
        case 1000000: return B1000000;
        case 1152000: return B1152000;
        case 1500000: return B1500000;
        case 2000000: return B2000000;
        default: return -1;
    }
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s <uart_port> <baudrate> [--verbose]\n", argv[0]);
        return -1;
    }
    const char* UART_PORT = argv[1];
    const int baudrate = getTermiosBadrate(atoi(argv[2]));
    if (baudrate == -1)
    {
        printf("Unsupported baudrate: %s\n", argv[2]);
        printf(
            "Supported baudrates: 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600, 1000000, 1152000, "
            "1500000, 2000000\n");
        return -1;
    }

    if (argc == 4 && strcmp(argv[3], "--verbose") == 0)
    {
        log_set_level(LOG_DEBUG);
    }
    else
    {
        log_set_level(LOG_INFO);
    }

    void* repSocket = zmqs_Init(ZMQ_PUB_ADDR, ZMQ_REP_ADDR);
    int uartFd = uart_Init(UART_PORT, baudrate);
    ctrl_Init();
    log_info("UART Server Started");

    int timerFd = timerfd_create(CLOCK_MONOTONIC, 0);
    struct itimerspec ts;
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 100000000;
    ts.it_value.tv_sec = 0;
    ts.it_value.tv_nsec = 100000000;
    timerfd_settime(timerFd, 0, &ts, NULL);

    zmq_pollitem_t items[3] = { 0 };
    items[0].socket = repSocket;
    items[0].events = ZMQ_POLLIN;
    items[1].fd = uartFd;
    items[1].events = ZMQ_POLLIN;
    items[2].fd = timerFd;
    items[2].events = ZMQ_POLLIN;

    while (1)
    {
        int rc = zmq_poll(items, 3, -1);
        if (rc == -1)
        {
            log_error("zmq_poll error: %s", zmq_strerror(zmq_errno()));
            break;
        }

        if (items[0].revents & ZMQ_POLLIN)
        {
            zmqs_HandleRequests((zmqs_Callbacks_t) { .varListGetter = ctrl_GetVarRegistry,
                                                     .onVarValueUpdate = ctrl_VarValueUpdateHandler,
                                                     .onTextInput = ctrl_SendTextInputHandler });
        }

        if (items[1].revents & ZMQ_POLLIN)
        {
            uart_Receive(ctrl_UartReceiveCallback);
        }

        if (items[2].revents & ZMQ_POLLIN)
        {
            uint64_t expirations;
            read(timerFd, &expirations, sizeof(expirations));
            ctrl_PeriodicTask();
        }
    }

    uart_Close();
    zmqs_Close();
    return 0;
}
