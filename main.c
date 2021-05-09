#include <stdio.h>
#include <string.h>

#include "socket.h"

void handle_client(SOCKET client);

int main() {
#ifdef _WIN32
    struct WSAData data;
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
        puts("Error initializing socket");
        goto exit;
    }
#endif

    struct addrinfo* result = NULL;
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP,
        .ai_flags = AI_PASSIVE
    };

    if (getaddrinfo(NULL, "1337", &hints, &result)) {
        puts("Error obtaining address info");
        goto exit;
    }

    SOCKET server;

    if ((server = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) < 0) {
        puts("Error obtaining socket");
        goto cleanup;
    }

    if (bind(server, result->ai_addr, result->ai_addrlen) < 0) {
        puts("Error binding socket");
        goto cleanup;
    }

    freeaddrinfo(result);

    if (listen(server, SOMAXCONN) < 0) {
        puts("Error listening on socket");
        goto cleanup;
    }

    for (;;) {
        SOCKET client;

        if ((client = accept(server, NULL, NULL)) < 0) {
            puts("Cannot accept client");
            goto cleanup;
        }

        handle_client(client);
        puts("Disconnecting client");

#ifdef _WIN32
        closesocket(client);
#else
        close(client);
#endif
    }

cleanup:
#ifdef _WIN32
    closesocket(server);
#else
    close(server);
#endif

exit:
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}

#include "buffer.h"
#include "buffer_socket.h"
#include "buffer_dynamic.h"

enum {
    PROTO_STATE_HANDSHAKE = 0,
    PROTO_STATE_STATUS = 1,
    PROTO_STATE_LOGIN = 2,
    PROTO_STATE_PLAY = 3
};

#define STRING0(_Value) #_Value
#define STRING(_Value) STRING0(_Value)

#define TRY(_Expr)                                                \
    do {                                                          \
        if (!(_Expr)) {                                           \
            puts("TRY failed in " __FILE__ ":" STRING(__LINE__)); \
            goto cleanup;                                         \
        }                                                         \
    } while (0)

int buffer_write_packet(struct buffer* buf, uint8_t id, const struct buffer* data) {
    struct buffer payload = { 0 };

    buffer_dynamic_init(&payload, 16);
    buffer_write_var32(&payload, id);
    buffer_write(&payload, buffer_dynamic_data(data), buffer_dynamic_size(data));

    if (!buffer_write_var32(buf, buffer_dynamic_size(&payload))) {
        buffer_free(&payload);
        return 0;
    }

    if (!buffer_write(buf, buffer_dynamic_data(&payload), buffer_dynamic_size(&payload))) {
        buffer_free(&payload);
        return 0;
    }

    buffer_free(&payload);
    return 1;
}

void handle_client(SOCKET client) {
    int state = PROTO_STATE_HANDSHAKE;
    struct buffer inbound_packet;

    buffer_socket_init(&inbound_packet, client);

    puts("New client connected");

    for (;;) {
        uint32_t packet_len = 0;
        uint32_t packet_id = 0;

        puts("Waiting for next packet to arrive");

        TRY(buffer_read_var32(&inbound_packet, &packet_len));
        TRY(buffer_read_var32(&inbound_packet, &packet_id));

        printf("Received new packet (id=%02x, len=%d, state=%d)\n", packet_id, packet_len, state);

        if (state == PROTO_STATE_HANDSHAKE && packet_id == 0x00) {
            uint32_t protocol_version;
            char server_address[255];
            uint16_t server_port;
            uint32_t next_state;

            memset(server_address, 0, sizeof(server_address));

            TRY(buffer_read_var32(&inbound_packet, &protocol_version));
            TRY(buffer_read_str(&inbound_packet, server_address, sizeof(server_address)));
            TRY(buffer_read_u16(&inbound_packet, &server_port));
            TRY(buffer_read_var32(&inbound_packet, &next_state));

            puts("Received client info");

            if (next_state != 1 && next_state != 2) {
                printf("Invalid next state: %d\n", next_state);
                goto cleanup;
            }

            state = next_state;
            continue;
        }

        if (state == PROTO_STATE_STATUS && packet_id == 0x00) {
            const char payload[] = "{\"version\":{\"name\":\"1.16.5\",\"protocol\":754},\"players\":{\"max\":1337,\"online\":0},\"description\":{\"text\":\"Some shitty Minecraft server written in C, indeed!\"}}";
            struct buffer packet;

            buffer_dynamic_init(&packet, sizeof(payload));
            buffer_write_str(&packet, payload);
            buffer_write_packet(&inbound_packet, 0x00, &packet);
            buffer_free(&packet);

            puts("Sent status response");
            continue;
        }

        if (state == PROTO_STATE_STATUS && packet_id == 0x01) {
            uint64_t payload;
            TRY(buffer_read_u64(&inbound_packet, &payload));

            struct buffer packet;
            buffer_dynamic_init(&packet, 8);
            buffer_write_u64(&packet, payload);
            buffer_write_packet(&inbound_packet, 0x01, &packet);
            buffer_free(&packet);

            puts("Sent pong response");
            goto cleanup;
        }

        if (state == PROTO_STATE_LOGIN && packet_id == 0x00) {
            char username[260] = { 0 };
            TRY(buffer_read_str(&inbound_packet, username, sizeof(username)));
            printf("Requested login as '%s'\n", username);

            const char payload[] = "{\"text\":\"LBGT members are not allowed to join this server.\"}";
            struct buffer packet;

            buffer_dynamic_init(&packet, sizeof(payload));
            buffer_write_str(&packet, payload);
            buffer_write_packet(&inbound_packet, 0x00, &packet);
            buffer_free(&packet);

            puts("Sent login response");
            goto cleanup;
        }

        puts("Unsupported packet");
        goto cleanup;
    }

cleanup:
    buffer_free(&inbound_packet);
}
