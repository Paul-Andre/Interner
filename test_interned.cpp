#include <stdio.h>
#include <chrono>
#include "Sharded.hpp"
#include <iostream>
#include <thread>
#include <vector>
using namespace std;
using std::vector;
using std::cout;
using std::endl;

typedef __uint64_t uint64_t;

SavedString *create_random_string(int max_len){
  // this is stupid. TODO remove random_scratch;
  char random_scratch[16];
  assert(max_len<=16);
  int len = 1+rand()%max_len;
  for (int i = 0; i<len; i++) {
    random_scratch[i] = 'a'+(rand()%26);
  }
  return new_saved_string(StringPiece(len, random_scratch));
}

vector<StringPiece> glob_strings;

Sharded s;
void thread_function(int x, int begin, int end) {
    std::cout << "Thread " << x<<" started." << std::endl;

    for (int i = begin; i<end; i++) {

      s.intern(glob_strings[i]);
      //cout << (a==c) <<endl;

      //cout<<d.value<<endl;
      //cout << (a==d) <<endl;
      if(i%(100*1000)==0) {
        std::cout << "Thread " << x<<" at iteration " << i<<"."<< std::endl;
      }
    }
    std::cout << "Thread " << x<<" ended." << std::endl;
}

vector<StringPiece> createRandom(int len) {
  vector<StringPiece> recurring;
  for (int i = 0; i<10*1000 ; i++) {
    SavedString *r = create_random_string(16);
    recurring.push_back(r);
  }

  vector<StringPiece> ret;
  for (int i = 0; i<len; i++) {
    int a = rand()%3;
    if (a ==0) {
      ret.push_back(new_saved_string(recurring[rand()%recurring.size()]));
    } else if (a ==1) {
      ret.push_back(create_random_string(16));
    } else {
      ret.push_back(create_random_string(2));
    }
  }
  return ret;

}


int main() {
  int len = 10*1000*1000;
  glob_strings=createRandom(len);
  assert(glob_strings.size() == len);

  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

  std::vector<std::thread> my_threads;
  int tot_threads = 16;
  for (int i=0; i<tot_threads; i++) {
    int begin = i*len/tot_threads;
    int end = (i+1)*len/tot_threads;
    my_threads.push_back(std::thread(thread_function, i, begin, end));
    //thread_function( i, begin, end);
  }

  for (auto &t: my_threads) {
    t.join();
  }

  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-begin;
  cout<<"Time elapsed: "<<elapsed_seconds.count()<<"s."<<endl;

  return 0;
}

//#define DEF_INTERN(name, str) {

  /*
DEF_INTERN(EQ, "==");
DEF_INTERN(IF, "if");

namespace to_be_interned {
  char *EQ = "==";
  char *IF = "==";
}

namespace pre {
  Token EQ = hash(to_be_interned.EQ);
  Token IF = hash(to_be_interned.IF);
}

run_pre_interning{Interner &interner}
{
  {
    char *interned = interner.intern(to_be_interned.EQ);
    assert(interned = to_be_interned.EQ);
  }
}

if (symbol == SYM.IF) {

}
*/
