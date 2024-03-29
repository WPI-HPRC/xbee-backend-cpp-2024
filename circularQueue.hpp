//
// Created by William Scheirey on 3/29/24.
//

#ifndef XBEE_BACKEND_CPP_CIRCULARQUEUE_HPP
#define XBEE_BACKEND_CPP_CIRCULARQUEUE_HPP

#include <cstdlib>
#include <cstring>
#include <iostream>

template<typename T>

struct CircularQueue
{
    unsigned int length;
    unsigned int dataSize_bytes;
    T *dataPtr;
    T *readPtr;
    T *data;
};

template<typename T>
inline CircularQueue<T> *serialCircularQueueCreate(unsigned int length)
{
    auto *buffer = (CircularQueue<T> *) malloc(sizeof(CircularQueue<T>));

    buffer->length = length;
    buffer->dataSize_bytes = sizeof(T);

    buffer->data = (uint8_t *) malloc(length * buffer->dataSize_bytes);

    buffer->dataPtr = &buffer->data[0];
    buffer->readPtr = buffer->dataPtr;

    std::cout << "Created circular queue with length " << std::dec << (int) buffer->length << " and data size = "
              << (int) buffer->dataSize_bytes << " bytes" << std::endl;

    return buffer;
}

template<typename T>
inline void serialCircularQueueRead(CircularQueue<T> *queue, T *outBuffer, unsigned int length)
{
    unsigned int length_bytes = length * queue->dataSize_bytes;

//    std::cout << "Attempting to read " << std::dec << (int) length_bytes << " bytes" << std::endl;

    if (queue->data + queue->length - queue->readPtr > length_bytes)
    {
//        std::cout << "Not at end of buffer. Read ptr = " << std::hex << (int) (*queue->readPtr & 0xFF) << std::endl;
        memcpy(outBuffer, queue->readPtr, length_bytes);
        queue->readPtr += length_bytes;
    }
    else
    {
        unsigned int bytes_transferred = queue->data + queue->length - queue->readPtr;

        memcpy(outBuffer, queue->readPtr, bytes_transferred);
        memcpy(&outBuffer[bytes_transferred], queue->data, length_bytes - bytes_transferred);

        queue->readPtr = &queue->data[bytes_transferred];
    }
}


template<typename T>
inline void serialCircularBufferAdd(CircularQueue<T> *buffer, T data)
{
//    std::cout << "Adding " << std::hex << (int) (data & 0xFF)
//              << std::endl;
    *buffer->dataPtr = data;
    if (buffer->dataPtr == &buffer->data[buffer->length - 1])
    {
        buffer->dataPtr = &buffer->data[0];
    }
    else
    {
        buffer->dataPtr++;
    }
}

#endif //XBEE_BACKEND_CPP_CIRCULARQUEUE_HPP
