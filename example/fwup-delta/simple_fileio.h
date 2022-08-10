#include <stddef.h>
#include <stdio.h>

#include "image.h"

typedef enum {
    SFIO_STREAM_SLOT,
    SFIO_STREAM_RAM,
} sfio_stream_type_t;

typedef struct {
    sfio_stream_type_t type;
    size_t offset;
    size_t size;
    union {
        uint8_t *ptr;
	image_slot_t slot;
    };
} sfio_stream_t;

size_t sfio_fread(void *ptr, size_t size, size_t count, sfio_stream_t *stream);

size_t sfio_fwrite(const void *ptr, size_t size, size_t count, sfio_stream_t *stream);

int sfio_fseek(sfio_stream_t *stream, long int offset, int whence);
