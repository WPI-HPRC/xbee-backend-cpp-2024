//
// Created by William Scheirey on 3/18/24.
//

#ifndef XBEE_BACKEND_CPP_CIRCULARBUFFER_HPP
#define XBEE_BACKEND_CPP_CIRCULARBUFFER_HPP

#include <cstdlib>
#include <cstring>
#include "qdebug.h"

#define CIRCULAR_BUFFER_LEN 10

typedef struct
{
    unsigned int length;
    int dataSize_bytes;
    uint8_t *dataPtr;
    uint8_t *data;
} SerialCircularBuffer;


inline uint8_t *serialCircularBufferGetValueAtIndex(SerialCircularBuffer *buffer, int index)
{
    return &(buffer->data[((buffer->dataPtr - buffer->data) + index * buffer->dataSize_bytes) %
                          (buffer->length * buffer->dataSize_bytes)]);
}

inline uint8_t *serialCircularBufferGetLastValue(SerialCircularBuffer *buffer)
{
    return serialCircularBufferGetValueAtIndex(buffer, -1);
}

inline SerialCircularBuffer *serialCircularBufferCreate(unsigned int length, unsigned int dataSize_bytes)
{
    auto *buffer = (SerialCircularBuffer *) malloc(dataSize_bytes * length);

    buffer->length = length;
    buffer->dataSize_bytes = dataSize_bytes;

    buffer->data = (uint8_t *) malloc(dataSize_bytes * length);

    buffer->dataPtr = &buffer->data[0];

    return buffer;
}

inline void serialCircularBufferAdd(SerialCircularBuffer *buffer, uint8_t *data, size_t length_bytes)
{
    memcpy(buffer->dataPtr, data, length_bytes);

    if (buffer->dataPtr == &buffer->data[buffer->length - 1])
    {
        buffer->dataPtr = &buffer->data[0];
        qDebug() << "Going back around";
    }
    else
    {
        buffer->dataPtr += buffer->dataSize_bytes;
    }
}

#endif //XBEE_BACKEND_CPP_CIRCULARBUFFER_HPP
