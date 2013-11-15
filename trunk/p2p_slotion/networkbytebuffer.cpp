/*
 * p2p solution
 * Copyright 2013, VZ Inc.
 * 
 * Author   : GuangLei He
 * Email    : guangleihe@gmail.com
 * Created  : 2013/11/15      17:09
 * Filename : F:\GitHub\trunk\p2p_slotion\networkbytebuffer.cpp
 * File path: F:\GitHub\trunk\p2p_slotion
 * File base: networkbytebuffer
 * File ext : cpp
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

#include "networkbytebuffer.h"

#include <algorithm>
#include <cassert>
#include <cstring>

#include "talk/base/basictypes.h"
#include "talk/base/byteorder.h"

static const int DEFAULT_SIZE = 4096;

NetworkByteBuffer::NetworkByteBuffer() {
  Construct(NULL, DEFAULT_SIZE, ORDER_NETWORK);
}

NetworkByteBuffer::NetworkByteBuffer(ByteOrder byte_order) {
  Construct(NULL, DEFAULT_SIZE, byte_order);
}

NetworkByteBuffer::NetworkByteBuffer(char* bytes, size_t len) {
  Construct(bytes, len, ORDER_NETWORK);
}

NetworkByteBuffer::NetworkByteBuffer(char* bytes, size_t len, ByteOrder byte_order) {
  Construct(bytes, len, byte_order);
}

NetworkByteBuffer::NetworkByteBuffer(char* bytes) {
  Construct(bytes, strlen(bytes), ORDER_NETWORK);
}

void NetworkByteBuffer::Construct(char* bytes, size_t len,
                           ByteOrder byte_order) 
{
  version_ = 0;
  start_ = 0;
  size_ = len;
  byte_order_ = byte_order;
  bytes_ = bytes;

  if (bytes) {
  end_ = len;
  memcpy(bytes_, bytes, end_);
  } else {
  end_ = 0;
  }
}

NetworkByteBuffer::~NetworkByteBuffer() {
  //delete[] bytes_;
}

bool NetworkByteBuffer::ReadUInt8(uint8* val) {
  if (!val) return false;

  return ReadBytes(reinterpret_cast<char*>(val), 1);
}

bool NetworkByteBuffer::ReadUInt16(uint16* val) {
  if (!val) return false;

  uint16 v;
  if (!ReadBytes(reinterpret_cast<char*>(&v), 2)) {
    return false;
  } else {
    *val = (byte_order_ == ORDER_NETWORK) ? talk_base::NetworkToHost16(v) : v;
    return true;
  }
}

bool NetworkByteBuffer::ReadUInt24(uint32* val) {
  if (!val) return false;

  uint32 v = 0;
  char* read_into = reinterpret_cast<char*>(&v);
  if (byte_order_ == ORDER_NETWORK || talk_base::IsHostBigEndian()) {
    ++read_into;
  }

  if (!ReadBytes(read_into, 3)) {
    return false;
  } else {
    *val = (byte_order_ == ORDER_NETWORK) ? talk_base::NetworkToHost32(v) : v;
    return true;
  }
}

bool NetworkByteBuffer::ReadUInt32(uint32* val) {
  if (!val) return false;

  uint32 v;
  if (!ReadBytes(reinterpret_cast<char*>(&v), 4)) {
    return false;
  } else {
    *val = (byte_order_ == ORDER_NETWORK) ? talk_base::NetworkToHost32(v) : v;
    return true;
  }
}

bool NetworkByteBuffer::ReadUInt64(uint64* val) {
  if (!val) return false;

  uint64 v;
  if (!ReadBytes(reinterpret_cast<char*>(&v), 8)) {
    return false;
  } else {
    *val = (byte_order_ == ORDER_NETWORK) ? talk_base::NetworkToHost64(v) : v;
    return true;
  }
}

bool NetworkByteBuffer::ReadString(std::string* val, size_t len) {
  if (!val) return false;

  if (len > Length()) {
    return false;
  } else {
    val->append(bytes_ + start_, len);
    start_ += len;
    return true;
  }
}

bool NetworkByteBuffer::ReadBytes(char* val, size_t len) {
  if (len > Length()) {
    return false;
  } else {
    memcpy(val, bytes_ + start_, len);
    start_ += len;
    return true;
  }
}

void NetworkByteBuffer::WriteUInt8(uint8 val) {
  WriteBytes(reinterpret_cast<const char*>(&val), 1);
}

void NetworkByteBuffer::WriteUInt16(uint16 val) {
  uint16 v = (byte_order_ == ORDER_NETWORK) ? talk_base::HostToNetwork16(val) : val;
  WriteBytes(reinterpret_cast<const char*>(&v), 2);
}

void NetworkByteBuffer::WriteUInt24(uint32 val) {
  uint32 v = (byte_order_ == ORDER_NETWORK) ? talk_base::HostToNetwork32(val) : val;
  char* start = reinterpret_cast<char*>(&v);
  if (byte_order_ == ORDER_NETWORK || talk_base::IsHostBigEndian()) {
    ++start;
  }
  WriteBytes(start, 3);
}

void NetworkByteBuffer::WriteUInt32(uint32 val) {
  uint32 v = (byte_order_ == ORDER_NETWORK) ? talk_base::HostToNetwork32(val) : val;
  WriteBytes(reinterpret_cast<const char*>(&v), 4);
}

void NetworkByteBuffer::WriteUInt64(uint64 val) {
  uint64 v = (byte_order_ == ORDER_NETWORK) ?talk_base::HostToNetwork64(val) : val;
  WriteBytes(reinterpret_cast<const char*>(&v), 8);
}

void NetworkByteBuffer::WriteString(const std::string& val) {
  WriteBytes(val.c_str(), val.size());
}

void NetworkByteBuffer::WriteBytes(const char* val, size_t len) {
  memcpy(ReserveWriteBuffer(len), val, len);
}

char* NetworkByteBuffer::ReserveWriteBuffer(size_t len) {
  if (Length() + len > Capacity())
    Resize(Length() + len);

  char* start = bytes_ + end_;
  end_ += len;
  return start;
}

void NetworkByteBuffer::Resize(size_t size) {
  size_t len = talk_base::_min(end_ - start_, size);
  if (size <= size_) {
    // Don't reallocate, just move data backwards
    memmove(bytes_, bytes_ + start_, len);
  } else {
    // Reallocate a larger buffer.
    size_ = talk_base::_max(size, 3 * size_ / 2);
    char* new_bytes = new char[size_];
    memcpy(new_bytes, bytes_ + start_, len);
    delete [] bytes_;
    bytes_ = new_bytes;
  }
  start_ = 0;
  end_ = len;
  ++version_;
}

bool NetworkByteBuffer::Consume(size_t size) {
  if (size > Length())
    return false;
  start_ += size;
  return true;
}

NetworkByteBuffer::ReadPosition NetworkByteBuffer::GetReadPosition() const {
  return ReadPosition(start_, version_);
}

bool NetworkByteBuffer::SetReadPosition(const ReadPosition &position) {
  if (position.version_ != version_) {
    return false;
  }
  start_ = position.start_;
  return true;
}

void NetworkByteBuffer::Clear() {
  memset(bytes_, 0, size_);
  start_ = end_ = 0;
  ++version_;
}