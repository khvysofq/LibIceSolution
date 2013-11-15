/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/15      17:10
 * Filename : F:\GitHub\trunk\p2p_slotion\networkbytebuffer.h
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: networkbytebuffer
 * File ext : h
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NETWORK_BYTE_BUFFER_H_
#define NETWORK_BYTE_BUFFER_H_

#include <string>

#include "talk/base/basictypes.h"
#include "talk/base/constructormagic.h"

class NetworkByteBuffer {
public:

  enum ByteOrder {
    ORDER_NETWORK = 0,  // Default, use network byte order (big endian).
    ORDER_HOST,         // Use the native order of the host.
  };

  // |byte_order| defines order of bytes in the buffer.
  NetworkByteBuffer();
  explicit NetworkByteBuffer(ByteOrder byte_order);
  NetworkByteBuffer(char* bytes, size_t len);
  NetworkByteBuffer(char* bytes, size_t len, ByteOrder byte_order);

  // Initializes buffer from a zero-terminated string.
  explicit NetworkByteBuffer(char* bytes);

  ~NetworkByteBuffer();

  const char* Data() const { return bytes_ + start_; }
  size_t Length() const { return end_ - start_; }
  size_t Capacity() const { return size_ - start_; }
  ByteOrder Order() const { return byte_order_; }

  // Read a next value from the buffer. Return false if there isn't
  // enough data left for the specified type.
  bool ReadUInt8(uint8* val);
  bool ReadUInt16(uint16* val);
  bool ReadUInt24(uint32* val);
  bool ReadUInt32(uint32* val);
  bool ReadUInt64(uint64* val);
  bool ReadBytes(char* val, size_t len);

  // Appends next |len| bytes from the buffer to |val|. Returns false
  // if there is less than |len| bytes left.
  bool ReadString(std::string* val, size_t len);

  // Write value to the buffer. Resizes the buffer when it is
  // neccessary.
  void WriteUInt8(uint8 val);
  void WriteUInt16(uint16 val);
  void WriteUInt24(uint32 val);
  void WriteUInt32(uint32 val);
  void WriteUInt64(uint64 val);
  void WriteString(const std::string& val);
  void WriteBytes(const char* val, size_t len);

  // Reserves the given number of bytes and returns a char* that can be written
  // into. Useful for functions that require a char* buffer and not a
  // NetworkByteBuffer.
  char* ReserveWriteBuffer(size_t len);

  // Resize the buffer to the specified |size|. This invalidates any remembered
  // seek positions.
  void Resize(size_t size);

  // Moves current position |size| bytes forward. Returns false if
  // there is less than |size| bytes left in the buffer. Consume doesn't
  // permanently remove data, so remembered read positions are still valid
  // after this call.
  bool Consume(size_t size);

  // Clears the contents of the buffer. After this, Length() will be 0.
  void Clear();

  // Used with GetReadPosition/SetReadPosition.
  class ReadPosition {
    friend class NetworkByteBuffer;
    ReadPosition(size_t start, int version)
      : start_(start), version_(version) { }
    size_t start_;
    int version_;
  };

  // Remembers the current read position for a future SetReadPosition. Any
  // calls to Shift or Resize in the interim will invalidate the position.
  ReadPosition GetReadPosition() const;

  // If the given position is still valid, restores that read position.
  bool SetReadPosition(const ReadPosition &position);

private:
  void Construct(char* bytes, size_t size, ByteOrder byte_order);

  char* bytes_;
  size_t size_;
  size_t start_;
  size_t end_;
  int version_;
  ByteOrder byte_order_;

  // There are sensible ways to define these, but they aren't needed in our code
  // base.
  DISALLOW_COPY_AND_ASSIGN(NetworkByteBuffer);
};

#endif