#pragma once

#include "basics.h"
#include "StringPiece.h"


namespace {
  bool _hm_has_capacity(ssize_t size, ssize_t capacity) {
    // This will exploit the x86_64 lea trick
    // return size * 2 < capacity;
    return size * 9 < capacity * 8;
  }

}


struct StringInterner {
  struct Bucket {
    hash_t hash;
    SavedString *data;
  };
  Bucket *buckets;
  ssize_t size;
  ssize_t capacity;

  StringInterner(): buckets(nullptr), size(0), capacity(0){}
  InternedToken intern(StringPiece p);
  void print();
};

namespace {

  typedef StringInterner::Bucket Bucket;

  struct StringInterner hm_with_capacity(ssize_t cap) {
    assert(cap > 0 && "cap needs to be positive");
    assert(cap-(cap&-cap) == 0 && "cap needs to be a power of two");
    struct StringInterner map;
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


  SavedString *new_saved_string(StringPiece p){
    ssize_t l = p.size;
    const char *s = p.data;
    SavedString *ret = (SavedString *)calloc(sizeof(ssize_t) + l, 1);
    ret->size = l;
    memcpy(ret->data, s, l);
    return ret;
  }

  InternedToken toToken(SavedString *s) {
    InternedToken ret;
    assert(unhash64(hash64((uint64_t)s)) == (uint64_t) s);
    ret.value = hash64((uint64_t) s);
    return ret;
  }

}

void  StringInterner::print() {
  auto map = this;
  for (uint64_t ii = 0; ii < map->capacity; ii++) {
    uint64_t i = (ii + 0) & (map->capacity - 1);
    if (map->buckets[i].hash == 0) {
      cout << "[0]     ";
    } else {
      cout << StringPiece(map->buckets[i].data) << "   ";
    }
  }
  cout <<endl;

}


InternedToken StringInterner::intern(StringPiece p){
  auto map = this;
  if (!_hm_has_capacity(map->size, map->capacity)) {
    _hm_increase_size(map);
  }

  hash_t h = hash_string(p);
  assert(h!=0);

  uint64_t h_trancated = h & (map->capacity - 1);

  SavedString *ret = NULL;
  for (uint64_t ii = 0; ii < map->capacity; ii++) {
    uint64_t i = (ii + h_trancated) & (map->capacity - 1);
    if (map->buckets[i].hash == 0) { // is empty
      map->buckets[i].hash = h;
      map->buckets[i].data = new_saved_string(p);
      map->size++;
      ret = map->buckets[i].data;
      break;
    } else if (map->buckets[i].hash == h) {
      if (map->buckets[i].data == p) {
        ret = map->buckets[i].data;
        break;
      }
      continue;
    } else {
      hash_t hh = map->buckets[i].hash;
      uint64_t hht = hh & (map->capacity - 1);
      uint64_t hh_diff = (i - hht) & (map->capacity - 1);
      if (ii > hh_diff) {
        _hm_shift_forward_until_empty(map, i);
        map->buckets[i].hash = h;
        map->buckets[i].data = new_saved_string(p);
        map->size++;
        ret = map->buckets[i].data;
        break;
      }
    }
  }
  assert(ret && "Could not find space in hash table to insert");
  return toToken(ret);
}

