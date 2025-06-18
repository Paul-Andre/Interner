#pragma once 

#include "basics.h"
#include "value.h"
#include <vector>
#include <cmath>
#include <iostream>
using std::vector;

namespace {

  ssize_t minSize = 2;
  ssize_t blockSize = 2;
  ssize_t growthRate = 2;

}

struct Memory {
  Value *areaA;
  Value *areaB;
  ssize_t sizeA;
  ssize_t sizeB;
  ssize_t ptr;

  ssize_t quantityLastTime;

  Memory() {
    areaA = (Value *)calloc(minSize, sizeof(Value));
    sizeA = minSize;

    areaB = (Value *)calloc(minSize, sizeof(Value));
    sizeB = minSize;

    ptr = 0;

    quantityLastTime = 0;
  }

  void print() {
    for (int i=0; i<sizeA; i++) {
      Value v = areaA[i];
      cout << "( "<<v.type << ", "<<v.value<<")  ";
    }
    cout <<endl;
  }
  void printB() {
    cout << "B: ";
    for (int i=0; i<sizeB; i++) {
      Value v = areaB[i];
      cout << "( "<<v.type << ", "<<v.value<<")  ";
    }
    cout <<endl;
  }
  void collect(vector<Value*> &anchors) {
    cerr << "collecting" << endl;
    if (sizeB < quantityLastTime*growthRate) {
      int newSize = std::max(minSize, quantityLastTime*growthRate);
      newSize = (newSize + (blockSize-1))/blockSize *blockSize;

      areaB = (Value *)calloc(newSize, sizeof(Value));
      sizeB = newSize;
      cerr<<"rresize " << newSize<<endl;
    }

    ssize_t read_ptr = 0;
    ssize_t write_ptr = 0;

    auto mov = [&](Value *v) {
      uint64_t mu = v->memoryUsed();
      if (mu > 0) {
        Value *pos = (Value *)v->value;
        Value *newPos;
        if (pos->type == tags::MOVED) {
          newPos = (Value *)pos->value;
        } else {
          newPos = areaB+write_ptr;
          memcpy(newPos, pos, mu*sizeof(Value));
          write_ptr += mu;
          pos->type = tags::MOVED;
          pos->value = (uint64_t) newPos;
        }
        v->value = (uint64_t) newPos;
      }
    };

    for (Value *v: anchors) {
      mov(v);
    }
    
    while(read_ptr < write_ptr) {
      mov(&areaB[read_ptr]);
      read_ptr++;
    }
    quantityLastTime = read_ptr;
    ptr = read_ptr;
    {
      auto swp = areaA;
      areaA = areaB;
      areaB = swp;
      auto sizeSwp = sizeA;
      sizeA = sizeB;
      sizeB = sizeSwp;
    }
    cerr << "collection finished" <<endl;
  };

  Value *allocate(ssize_t alloc_size, vector<Value*> &anchors) {
    if (ptr+alloc_size > sizeA) {
      collect(anchors);
    }
    if (ptr+alloc_size > sizeA) {
      collect(anchors);
    }
    Value *ret = areaA+ptr;
    ptr += alloc_size;
    return ret;
  }


  Value makePair(Value a, Value b, vector<Value*> &anchors) {
    anchors.push_back(&a);
    anchors.push_back(&b);
    Value *location = allocate(2, anchors);
    location[0] = a;
    location[1] = b;
    Value ret;
    ret.type = tags::PAIR;
    ret.value = (uint64_t) location;
    anchors.pop_back();
    anchors.pop_back();
    return ret;
  }
};

