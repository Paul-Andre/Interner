#pragma once

#include "basics.h"

namespace tags {
  const uint64_t NONE = 1;
  const uint64_t BOOL = 2;
  const uint64_t SYMBOL = 3;
  const uint64_t FLOAT = 4;
  const uint64_t EMPTY_LIST = 5;
  const uint64_t PAIR = 6;

  const uint64_t MOVED = 1234123;
};


struct Value {
  uint64_t type;
  uint64_t value;
  // memoryUsed, in terms of sizeof(Value
  uint64_t memoryUsed() {
    assert(type != tags::MOVED);
    if (type == tags::PAIR) {
      return 2;
    }
    return 0;
  }

  bool isList() {
    return type==tags::EMPTY_LIST || type==tags::PAIR;
  }

  Value car() {
    assert(type==tags::PAIR);
    return ((Value *) value)[0];
  }
  Value cdr() {
    assert(type==tags::PAIR);
    return ((Value *) value)[1];
  }

};


namespace literal {
  const Value NONE = {tags::NONE, 0};
  const Value FALSE = {tags::BOOL, 0};
  const Value TRUE = {tags::BOOL, 1};
};
