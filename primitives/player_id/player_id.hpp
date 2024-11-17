#pragma once


#include <sys/socket.h>


struct PlayerId {
    sockaddr_storage IpAddr;
    int ConnSockFd;
};
