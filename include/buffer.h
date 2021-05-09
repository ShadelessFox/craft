#ifndef CRAFT_BUFFER_H
#define CRAFT_BUFFER_H

#include <stdint.h>
#include <stdlib.h>

struct buffer {
    void* data;
    int (*read)(struct buffer*, void*, size_t);
    int (*write)(struct buffer*, const void*, size_t);
    void (*free)(struct buffer*);
};

int buffer_read(struct buffer*, void*, size_t);
int buffer_read_i8(struct buffer*, int8_t*);
int buffer_read_u8(struct buffer*, uint8_t*);
int buffer_read_i16(struct buffer*, int16_t*);
int buffer_read_u16(struct buffer*, uint16_t*);
int buffer_read_i32(struct buffer*, int32_t*);
int buffer_read_u32(struct buffer*, uint32_t*);
int buffer_read_i64(struct buffer*, int64_t*);
int buffer_read_u64(struct buffer*, uint64_t*);
int buffer_read_str(struct buffer*, char*, size_t);
int buffer_read_var32(struct buffer*, uint32_t*);
int buffer_read_var64(struct buffer*, uint64_t*);

int buffer_write(struct buffer*, const void*, size_t);
int buffer_write_i8(struct buffer*, int8_t);
int buffer_write_u8(struct buffer*, uint8_t);
int buffer_write_i16(struct buffer*, int16_t);
int buffer_write_u16(struct buffer*, uint16_t);
int buffer_write_i32(struct buffer*, int32_t);
int buffer_write_u32(struct buffer*, uint32_t);
int buffer_write_i64(struct buffer*, int64_t);
int buffer_write_u64(struct buffer*, uint64_t);
int buffer_write_str(struct buffer*, const char*);
int buffer_write_var32(struct buffer*, uint32_t);
int buffer_write_var64(struct buffer*, uint64_t);

void buffer_free(struct buffer*);

#endif //CRAFT_BUFFER_H
