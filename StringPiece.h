#pragma once

#include <cstring>
#include <string>
#include <iostream>
#include "hash.h"
using std::string;
using std::ostream;

struct SavedString{
  ssize_t size;
  char data[];
};

struct InternedToken {
  hash_t value;
  bool operator==(const InternedToken &other) const{
    return value == other.value;
  }
  bool operator!=(const InternedToken &other) const{
    return value != other.value;
  }
  hash_t hash() const{
    return value;
  }
};

struct StringPiece{
  ssize_t size;
  const char *data;

  StringPiece(ssize_t s, const char* c) {
    size = s;
    data = c;
  }
  StringPiece(const SavedString *s) {
    data = s->data;
    size = s->size;
  }
  StringPiece(const string &s) {
    size = s.size();
    data = s.c_str();
  }
  StringPiece(const char* c) {
    size = std::strlen(c);
    data = c;
  }
  StringPiece(InternedToken p) {
    auto s = (SavedString *)unhash64(p.value);
    data = s->data;
    size = s->size;
  }
};
ostream& operator<<(ostream& ioOut, StringPiece s) {
  ioOut.write(s.data, s.size);
  return ioOut;
}
bool operator==(StringPiece a, StringPiece b){
  return (a.size == b.size && memcmp(a.data, b.data, a.size) ==0 );
}
bool operator!=(StringPiece a, StringPiece b){
  return !(a==b);
}

hash_t hash_string(StringPiece p){
  // The hash returned might be zero.
  ssize_t l=p.size;
  const char *s = p.data;
  hash_t a = (hash_t)(0) - (hash_t)(1);
  a = acc_hash(a,l);
  for(int i=0; i<l; i++) {
    a = acc_hash(a, s[i]);
  }
  return a;
}
