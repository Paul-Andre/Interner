//#pragma once
#include <bits/stdc++.h>

using namespace std;

#include "basics.h"
#include "StringPiece.h"


typedef uint64_t hash_t;
typedef uint64_t Key;
typedef uint64_t Value;

struct SavedString{
  ssize_t size;
  char data[];
};

struct Bucket {
  hash_t hash;
  SavedString *data;
};

struct StringInterner {
  Bucket *buckets;
  ssize_t size;
  ssize_t capacity;
};

hash_t hash64(uint64_t x) {
  //x *= 145;
  x ^= x >> 32;
  x *= UINT64_C(0xd6e8feb86659fd93);
  x ^= x >> 32;
  x *= UINT64_C(0xd6e8feb86659fd93);
  x ^= x >> 32;
  // return x & 0xff;
  return x;
}

hash_t acc_hash(hash_t ha, uint64_t x) {
  return hash64(ha ^ x);
}

hash_t hash_string(ssize_t l, const char *s) {
  // note that this will return a hash of 0 only if l is 2^64-1 and the string
  // is all zeros.
  hash_t a = (hash_t)(0) - (hash_t)(1);
  a = acc_hash(a,l);
  for(int i=0; i<l; i++) {
    a = acc_hash(a, s[l]);
  }
  return a;
}


bool _hm_has_capacity(ssize_t size, ssize_t capacity) {
  // This will exploit the x86_64 lea trick
  // return size * 2 < capacity;
  return size * 9 < capacity * 8;
}

struct StringInterner hm_with_capacity(ssize_t cap) {
  assert(cap > 0 || "cap needs to be positive");
  assert(cap-(cap&-cap) == 0 || "cap needs to be a power of two");
  struct StringInterner map = {0};
  if (cap != 0) {
    map.capacity = cap;
    map.buckets = (Bucket*)calloc(cap, sizeof(Bucket));
  }
  return map;
}

void _hm_shift_forward(struct StringInterner *map, uint64_t first,
                       uint64_t to_overwrite) {

  assert(to_overwrite != first);
  if (to_overwrite > first) {
    uint64_t distance = to_overwrite - first;
    memcpy(&map->buckets[first + 1], &map->buckets[first],
           distance * sizeof(Bucket));
  } else {
    memcpy(&map->buckets[1], &map->buckets[0], to_overwrite * sizeof(Bucket));

    map->buckets[0] = map->buckets[map->capacity - 1];

    uint64_t distance = map->capacity - first - 1;
    memcpy(&map->buckets[first + 1], &map->buckets[first],
           distance * sizeof(Bucket));
  }
}

// Shifts until finds empty space
void _hm_shift_forward_until_empty(struct StringInterner *map, uint64_t pos) {
  uint64_t ii = 0;
  bool found = false;
  uint64_t first_empty;
  for (; ii < map->capacity; ii++) {
    uint64_t i = (ii + pos) & (map->capacity - 1);
    if (map->buckets[i].hash == 0) {
      first_empty = i;
      found = true;
      break;
    }
  }
  assert(found);

  _hm_shift_forward(map, pos, first_empty);
}

bool strings_equal(SavedString *a, ssize_t l, const char *b) {
  return (a->size == l && memcmp(a->data, b, l)==0);
}


void _hm_reinsert (StringInterner *map, Bucket b){
  uint64_t h_trancated = b.hash & (map->capacity - 1);

  for (uint64_t ii = 0; ii < map->capacity; ii++) {
    uint64_t i = (ii + h_trancated) & (map->capacity - 1);
    if (map->buckets[i].hash == 0) { // is empty
      map->buckets[i] = b;
      return;
    } else if (map->buckets[i].hash == b.hash) {
      continue;
    } else {
      hash_t hh = map->buckets[i].hash;
      uint64_t hht = hh & (map->capacity - 1);
      uint64_t hh_diff = (i - hht) & (map->capacity - 1);
      if (ii > hh_diff) {
        _hm_shift_forward_until_empty(map, i);
        map->buckets[i] = b;
      }
    }
  }
}

void _hm_increase_size(struct StringInterner *map) {
  if (map->capacity == 0) {
    *map = hm_with_capacity(2);
    return;
  } else {
    struct StringInterner newMap = hm_with_capacity(map->capacity * 2);
    for (int i = 0; i < map->capacity; i++) {
      if (map->buckets[i].hash != 0) {
        _hm_reinsert(&newMap, map->buckets[i]);
      }
    }
    newMap.size = map->size;
    *map = newMap;
  }
}


SavedString *new_saved_string(ssize_t l, const char *s){
  SavedString *ret = (SavedString *)calloc(sizeof(ssize_t) + l, 1);
  ret->size = l;
  memcpy(ret->data, s, l);
  return ret;
}


SavedString *intern_string(StringInterner *map, ssize_t l, const char *s) {
  if (!_hm_has_capacity(map->size, map->capacity)) {
    _hm_increase_size(map);
  }

  hash_t h = hash_string(l, s);
  assert(h!=0);

  uint64_t h_trancated = h & (map->capacity - 1);

  SavedString *ret = NULL;
  for (uint64_t ii = 0; ii < map->capacity; ii++) {
    uint64_t i = (ii + h_trancated) & (map->capacity - 1);
    if (map->buckets[i].hash == 0) { // is empty
      map->buckets[i].hash = h;
      map->buckets[i].data = new_saved_string(l, s);
      map->size++;
      return map->buckets[i].data;
    } else if (map->buckets[i].hash == h) {
      cerr << "collision\n";
      if (strings_equal(map->buckets[i].data, l, s)) {
        cerr << "found old\n";
        return map->buckets[i].data;
      }
      continue;
    } else {
      hash_t hh = map->buckets[i].hash;
      uint64_t hht = hh & (map->capacity - 1);
      uint64_t hh_diff = (i - hht) & (map->capacity - 1);
      if (ii > hh_diff) {
        _hm_shift_forward_until_empty(map, i);
        map->buckets[i].hash = h;
        map->buckets[i].data = new_saved_string(l, s);
        map->size++;
        return map->buckets[i].data;
      }
    }
  }
  assert(false && "Could not find space in hash table to insert");
}

SavedString *intern_string(StringInterner *map, string s) {
  return intern_string(map, s.size(), s.c_str());
}

void printSavedString(SavedString *s) {
  for (int i=0; i<s->size; i++) {
    cout << s->data[i];
  }
}

int main() {
  cout <<hash64(0) << endl;
  StringInterner interner = {0};
  SavedString *ss1;
  for (int i=0; i<5; i++) {
    SavedString *ss = intern_string(&interner, "hello");
    cout << ss << endl;
    printSavedString(ss);
    ss1 = ss;
  }
  cout <<endl;
  for (int i=0; i<5; i++) {
    SavedString *ss = intern_string(&interner, "goody");
    cout << ss << endl;
    printSavedString(ss);
  }
    printSavedString(ss1);
  cout <<6-(6&-6) <<endl;
  return 0;
}

