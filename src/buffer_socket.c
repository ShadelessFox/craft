#include "buffer.h"
#include "buffer_socket.h"

static int buffer_socket_read(struct buffer*, void*, size_t);
static int buffer_socket_write(struct buffer*, const void*, size_t);
static void buffer_socket_free(struct buffer*);

int buffer_socket_init(struct buffer* buf, SOCKET sock) {
    buf->data = (void*)sock;
    buf->read = buffer_socket_read;
    buf->write = buffer_socket_write;
    buf->free = buffer_socket_free;
    return 1;
}

int buffer_socket_read(struct buffer* buf, void* ptr, size_t len) {
    int result = recv((SOCKET)buf->data, (char*)ptr, len, 0);
    return result >= 0 && result == len;
}

int buffer_socket_write(struct buffer* buf, const void* ptr, size_t len) {
    int result = send((SOCKET)buf->data, (const char*)ptr, len, 0);
    return result >= 0 && result == len;
}

void buffer_socket_free(struct buffer* buf) {
    buf->data = NULL;
    buf->read = NULL;
    buf->write = NULL;
    buf->free = NULL;
}