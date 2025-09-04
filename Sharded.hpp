#pragma once
#include <mutex>
#include <condition_variable>
#include "Interner.h"


struct Shard {

  // Number of readers (all reading simultaneously)
  int num_readers = 0;
  std::mutex num_readers_mutex;
  std::condition_variable num_readers_zero;

  // Number of writers (the one currently writing + the ones queued)
  int num_writers = 0;
  std::mutex num_writers_mutex;
  std::condition_variable num_writers_zero;

  // Mutex for the one thread currently writing.
  std::mutex writing_mutex;

  StringInterner interner;


  InternedToken _intern(StringPiece p, hash_t h) {

    {
      std::unique_lock nw_lk(num_writers_mutex);
      while(num_writers>0){
        num_writers_zero.wait(nw_lk);
      }
      {
        std::unique_lock nr_lk(num_readers_mutex);
        num_readers += 1;
      }
    }

    InternedToken value = interner.find(p, h);

    {
      std::unique_lock nr_lk(num_readers_mutex);
      num_readers -= 1;
      if (num_readers == 0) {
        num_readers_zero.notify_one();
      }
    }
    if (value) {
      return value;
    }
    {
      std::unique_lock nr_rk(num_writers_mutex);
      num_writers+=1;
    }
    {
      std::unique_lock nr_lk(num_readers_mutex);
      while(num_readers>0) {
        num_readers_zero.wait(nr_lk);
      }
    }
    {
      std::unique_lock w_lk(writing_mutex);

      value = interner.intern(p, h);

      num_readers_zero.notify_one();  // Wake up next writing thread.
    }
    {
      std::unique_lock nr_lk(num_writers_mutex);
      num_writers -= 1;
      if (num_writers == 0) {
        num_writers_zero.notify_all();
      }
    }
    return value;
  }

  InternedToken intern(StringPiece p) {
    hash_t h = hash_string(p);
    return _intern(p,h);
  }
};


struct Sharded {
  static constexpr int num_sharding_bits = 11;

  Shard shards[1<<num_sharding_bits];

  // methods
  InternedToken intern(StringPiece p){
    hash_t h = hash_string(p);
    uint64_t index = h>>(64-num_sharding_bits);
    //std::cout<<"Shard: "<<index<<std::endl;
    return shards[index]._intern(p, h);
  }
};



  

