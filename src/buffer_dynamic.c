#include "buffer.h"
#include "buffer_dynamic.h"

#include <assert.h>
#include <malloc.h>
#include <string.h>

struct buf_dynamic_data {
    uint8_t* set;
    uint8_t* cur;
    uint8_t* end;
};

static int buffer_dynamic_read(struct buffer*, void*, size_t);
static int buffer_dynamic_write(struct buffer*, const void*, size_t);
static void buffer_dynamic_free(struct buffer*);

int buffer_dynamic_init(struct buffer* buf, size_t cap) {
    struct buf_dynamic_data* data = calloc(1, sizeof(struct buf_dynamic_data));
    data->set = data->cur = malloc(cap);
    data->end = data->set + cap;

    buf->data = data;
    buf->read = buffer_dynamic_read;
    buf->write = buffer_dynamic_write;
    buf->free = buffer_dynamic_free;
    return 1;
}

int buffer_dynamic_seek(struct buffer* buf, size_t pos) {
    struct buf_dynamic_data* data = (struct buf_dynamic_data*)buf->data;
    assert(data->set + pos <= data->end && "Seek position is too big");
    data->cur = data->set + pos;
    return 1;
}

size_t buffer_dynamic_size(const struct buffer* buf) {
    struct buf_dynamic_data* data = (struct buf_dynamic_data*)buf->data;
    return data->cur - data->set;
}

void* buffer_dynamic_data(const struct buffer* buf) {
    struct buf_dynamic_data* data = (struct buf_dynamic_data*)buf->data;
    return data->set;
}

int buffer_dynamic_read(struct buffer* buf, void* ptr, size_t len) {
    struct buf_dynamic_data* data = (struct buf_dynamic_data*)buf->data;
    if (data->end - data->cur <= len) {
        return 0;
    }
    memcpy(ptr, data->cur, len);
    return 1;
}

int buffer_dynamic_write(struct buffer* buf, const void* ptr, size_t len) {
    struct buf_dynamic_data* data = (struct buf_dynamic_data*)buf->data;

    if (data->end - data->cur <= len) {
        size_t old_cap = (data->end - data->set);
        size_t old_cur = (data->cur - data->set);
        size_t new_cap = old_cap;

        while (new_cap < old_cap + len) {
            new_cap <<= 1;
        }

        data->set = realloc(data->set, new_cap);
        data->cur = data->set + old_cur;
        data->end = data->set + new_cap;
    }

    memcpy(data->cur, ptr, len);
    data->cur += len;
    return 1;
}

void buffer_dynamic_free(struct buffer* buf) {
    struct buf_dynamic_data* data = (struct buf_dynamic_data*)buf->data;
    free(data->set);
    free(data);
    buf->data = NULL;
    buf->read = NULL;
    buf->write = NULL;
    buf->free = NULL;
}