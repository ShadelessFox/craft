#include "buffer.h"

#include <string.h>

#define DECLARE_READ_GENERIC(_Name, _Type) \
    int buffer_read_##_Name(struct buffer* buf, _Type* val) { return buffer_read(buf, val, sizeof(_Type)); }

#define DECLARE_WRITE_GENERIC(_Name, _Type) \
    int buffer_write_##_Name(struct buffer* buf, _Type val) { return buffer_write(buf, &val, sizeof(_Type)); }

DECLARE_READ_GENERIC(i8, int8_t)
DECLARE_READ_GENERIC(u8, uint8_t)
DECLARE_READ_GENERIC(i16, int16_t)
DECLARE_READ_GENERIC(u16, uint16_t)
DECLARE_READ_GENERIC(i32, int32_t)
DECLARE_READ_GENERIC(u32, uint32_t)
DECLARE_READ_GENERIC(i64, int64_t)
DECLARE_READ_GENERIC(u64, uint64_t)

DECLARE_WRITE_GENERIC(i8, int8_t)
DECLARE_WRITE_GENERIC(u8, uint8_t)
DECLARE_WRITE_GENERIC(i16, int16_t)
DECLARE_WRITE_GENERIC(u16, uint16_t)
DECLARE_WRITE_GENERIC(i32, int32_t)
DECLARE_WRITE_GENERIC(u32, uint32_t)
DECLARE_WRITE_GENERIC(i64, int64_t)
DECLARE_WRITE_GENERIC(u64, uint64_t)

int buffer_read(struct buffer* buf, void* ptr, size_t len) {
    return buf->read && buf->read(buf, ptr, len);
}

int buffer_read_str(struct buffer* buf, char* ptr, size_t ptr_len) {
    uint32_t len;
    return buffer_read_var32(buf, &len) && (len <= ptr_len) && buffer_read(buf, ptr, len);
}

int buffer_read_var32(struct buffer* buf, uint32_t* ptr) {
    uint32_t res = 0;
    uint8_t len = 0;
    uint8_t val;

    for (;;) {
        if (!buffer_read_u8(buf, &val)) {
            return 0;
        }

        res |= (val & 0b1111111) << (7 * len);
        len += 1;

        if (len > 5) {
            return 0;
        }

        if ((val & 0b10000000) == 0) {
            *ptr = res;
            return 1;
        }
    }
}

int buffer_read_var64(struct buffer* buf, uint64_t* ptr) {
    uint64_t res = 0;
    uint8_t len = 0;
    uint8_t val;

    for (;;) {
        if (!buffer_read_u8(buf, &val)) {
            return 0;
        }

        res |= (val & 0b1111111) << (7 * len);
        len += 1;

        if (len > 10) {
            return 0;
        }

        if ((val & 0b10000000) == 0) {
            *ptr = res;
            return 1;
        }
    }
}

int buffer_write(struct buffer* buf, const void* ptr, size_t len) {
    return buf->write && buf->write(buf, ptr, len);
}

int buffer_write_str(struct buffer* buf, const char* str) {
    size_t len = strlen(str);
    return buffer_write_var32(buf, len) && buffer_write(buf, str, len);
}

int buffer_write_var32(struct buffer* buf, uint32_t val) {
    return buffer_write_var64(buf, val);
}

int buffer_write_var64(struct buffer* buf, uint64_t val) {
    uint8_t tmp;

    for (;;) {
        tmp = (uint8_t)(val & 0b1111111);
        val >>= 7;

        if (val != 0) {
            tmp |= 0b10000000;
        }

        if (!buffer_write_u8(buf, tmp)) {
            return 0;
        }

        if (val == 0) {
            return 1;
        }
    }
}

void buffer_free(struct buffer* buf) {
    if (buf->free) {
        buf->free(buf);
    }
}