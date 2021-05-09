#ifndef CRAFT_SOCKET_H
#define CRAFT_SOCKET_H

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netdb.h>
    #include <unistd.h>

    typedef int SOCKET;
#endif

#endif //CRAFT_SOCKET_H
