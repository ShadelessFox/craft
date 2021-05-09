#ifndef CRAFT_BUFFER_DYNAMIC_H
#define CRAFT_BUFFER_DYNAMIC_H

#include "buffer.h"

int buffer_dynamic_init(struct buffer*, size_t);
int buffer_dynamic_seek(struct buffer*, size_t);
size_t buffer_dynamic_size(const struct buffer*);
void* buffer_dynamic_data(const struct buffer*);

#endif //CRAFT_BUFFER_DYNAMIC_H
