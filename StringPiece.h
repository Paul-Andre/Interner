#pragma once

#include <cstring>
#include <string>
#include <iostream>
#include "hash.h"
#include <sstream>
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
  explicit operator bool() const {
    // Assumes the hash of zero is zero.
    return value != 0;
  }
};

struct StringPiece;
ostream& operator<<(ostream& ioOut, StringPiece s);

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
  explicit operator std::string() {
    std::ostringstream os;
    os<<(*this);
    return os.str();
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

// mallocs a new SavedString
SavedString *new_saved_string(StringPiece p){
  ssize_t l = p.size;
  const char *s = p.data;
  SavedString *ret = (SavedString *)calloc(sizeof(ssize_t) + l, 1);
  ret->size = l;
  memcpy(ret->data, s, l);
  return ret;
}

hash_t hash_string(StringPiece p){
  ssize_t l=p.size;
  const char *s = p.data;
  hash_t a = (hash_t)(0) - (hash_t)(1);
  a = acc_hash(a,l);
  for(int i=0; i<l; i++) {
    a = acc_hash(a, s[i]);
  }
  if (a == 0) {
    a = (1ull << 63);
  }
  return a;
}
