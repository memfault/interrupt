#include <assert.h>
#include <string.h>

#include "simple_fileio.h"
#include "dfu.h"

size_t sfio_fread(void *ptr, size_t size, size_t count, sfio_stream_t *stream) {
    assert(size == 1); 
    if (stream->offset + count > stream->size) {
        count = stream->size - stream->offset;
    }
    if (stream->type == SFIO_STREAM_SLOT) {
        dfu_read(stream->slot, ptr, stream->offset, size * count);
    } else {
        memcpy(ptr, stream->ptr + stream->offset, size * count);
    }

    return count * size;
}

size_t sfio_fwrite(const void *ptr, size_t size, size_t count, sfio_stream_t *stream) {
    assert(size == 1); 
    if (stream->offset + count > stream->size) {
        count = stream->size - stream->offset;
    }
    if (stream->type == SFIO_STREAM_SLOT) {
        dfu_write(stream->slot, ptr, stream->offset, size * count);
    } else {
        memcpy(stream->ptr + stream->offset, ptr, size * count);
    }

    return count * size;
}

int sfio_fseek(sfio_stream_t *stream, long int offset, int origin) {
    assert(origin == SEEK_SET);
    if (offset > stream->size) {
        return -1;
    } else {
        stream->offset = offset; 
    }
    return 0;
}
