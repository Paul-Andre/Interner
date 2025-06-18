#pragma once

typedef uint64_t hash_t;

hash_t hash64(uint64_t x) {
  //x *= 145;
  x ^= x >> 32;
  x *= UINT64_C(0xd6e8feb86659fd93);
  x ^= x >> 32;
  x *= UINT64_C(14982988110149298331);
  x ^= x >> 32;
  // return x & 0xff;
  return x;
}

uint64_t unhash64(hash_t y) {
  return hash64(y);
}

hash_t acc_hash(hash_t ha, uint64_t x) {
  return hash64(ha ^ x);
}

