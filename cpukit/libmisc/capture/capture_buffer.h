/**
 * @file capture_buffer.h
 *
 * @brief Capture buffer
 *
 * This is a set of functions to control a variable length capture record buffer.
 */

/*
  ------------------------------------------------------------------------

  COPYRIGHT (c) 2014.
  On-Line Applications Research Corporation (OAR).

  Copyright 2016 Chris Johns <chrisj@rtems.org>.
  All rights reserved.

  The license and distribution terms for this file may be
  found in the file LICENSE in this distribution.

  This software with is provided ``as is'' and with NO WARRANTY.

  ------------------------------------------------------------------------
*/

#ifndef __CAPTURE_BUFFER_H_
#define __CAPTURE_BUFFER_H_

#include <stdlib.h>

/**@{*/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Capture buffer. There is one per CPU.
 */
typedef struct rtems_capture_buffer {
  uint8_t* buffer;       /**< The per cpu buffer. */
  size_t   size;         /**< The size of the buffer in bytes. */
  size_t   count;        /**< The number of used bytes in the buffer. */
  size_t   head;         /**< First record. */
  size_t   tail;         /**< Head == Tail for empty. */
  size_t   end;          /**< Buffer current end, it may move in. */
  size_t   max_rec;      /**< The largest record in the buffer. */
} rtems_capture_buffer;

/**
 * CTF Packet header. Required at the start of the
 * buffer.
 */
typedef struct rtems_capture_ctf_packet_header {
  uint32_t magic;
  uint32_t stream_id;
} rtems_capture_ctf_packet_header;

/**
 * CTF Packer context. Required at the start of the
 * buffer, after the header.
 */
typedef struct rtems_capture_ctf_packet_context {
  uint32_t content_size;
  uint32_t cpu_id;
} rtems_capture_ctf_packet_context;

static inline void
rtems_capture_buffer_flush (rtems_capture_buffer* buffer)
{
  buffer->end = buffer->size;
  buffer->count = 0;
  buffer->max_rec = 0;
}

static inline void
rtems_capture_buffer_create (rtems_capture_buffer* buffer, size_t size, uint32_t cpu_id)
{
  uint32_t ctf_header_size = sizeof(rtems_capture_ctf_packet_header) +
                             sizeof(rtems_capture_ctf_packet_context);
  rtems_capture_ctf_packet_header*  ctf_header = NULL;
  rtems_capture_ctf_packet_context* ctf_context = NULL;

  buffer->buffer = malloc(size + ctf_header_size);

  ctf_header = (rtems_capture_ctf_packet_header*)buffer->buffer;
  ctf_header->magic = 0xC1FC1FC1;
  ctf_header->stream_id = 0;

  ctf_context = (rtems_capture_ctf_packet_context*)(buffer->buffer +
                 sizeof(rtems_capture_ctf_packet_header));
  ctf_context->content_size = ctf_header_size << 8;
  ctf_context->cpu_id = cpu_id;

  /* This can work because CTF events within a packet are not expected to be
   * ordered by timestamp. Even if the buffer wraps, the packet header
   * and context are still in valid positions. 
   * */
  buffer->buffer = buffer->buffer + ctf_header_size;

  buffer->size = size;
  rtems_capture_buffer_flush (buffer);
}

static inline void
rtems_capture_buffer_destroy (rtems_capture_buffer*  buffer)
{
  rtems_capture_buffer_flush (buffer);
  free (buffer->buffer);
  buffer->buffer = NULL;
}

static inline bool
rtems_capture_buffer_is_empty (rtems_capture_buffer* buffer)
{
   return buffer->count == 0;
}

static inline bool
rtems_capture_buffer_is_full (rtems_capture_buffer* buffer)
{
   return buffer->count == buffer->size;
}

static inline bool
rtems_capture_buffer_has_wrapped (rtems_capture_buffer* buffer)
{
  if (buffer->tail > buffer->head)
    return true;

  return false;
}

static inline void*
rtems_capture_buffer_peek (rtems_capture_buffer* buffer, size_t* size)
{
  if (rtems_capture_buffer_is_empty (buffer))
  {
    *size = 0;
    return NULL;
  }

  if (buffer->tail > buffer->head)
    *size = buffer->end - buffer->tail;
  else
    *size = buffer->head - buffer->tail;

  return &buffer->buffer[buffer->tail];
}

void* rtems_capture_buffer_allocate (rtems_capture_buffer* buffer, size_t size);

void* rtems_capture_buffer_free (rtems_capture_buffer* buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif
