#include <stdio.h>
#include <chrono>
#include "Sharded.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <unordered_map>
#include <map>
using namespace std;
using std::vector;
using std::cout;
using std::endl;

typedef __uint64_t uint64_t;

SavedString *create_random_string(int max_len, bool caps=false){
  // this is stupid. TODO remove random_scratch;
  char random_scratch[16];
  assert(max_len<=16);
  int len = 1+rand()%max_len;
  for (int i = 0; i<len; i++) {
    random_scratch[i] = (caps?'A':'a')+(rand()%26);
  }
  return new_saved_string(StringPiece(len, random_scratch));
}

vector<StringPiece> in_strings;
vector<InternedToken> out_strings;

Sharded s;
void thread_function(int x, int begin, int end) {
    std::cout << "Thread " << x<<" started." << std::endl;

    for (int i = begin; i<end; i++) {

      out_strings[i] = s.intern(in_strings[i]);
      //cout << (a==c) <<endl;
      assert(out_strings[i] == in_strings[i]);
      //cout << out_strings[i] << " " << in_strings[i] <<endl;

      //cout<<d.value<<endl;
      //cout << (a==d) <<endl;
      if(i%(100*1000)==0) {
        std::cout << "Thread " << x<<" at iteration " << i<<"."<< std::endl;
      }
    }
    std::cout << "Thread " << x<<" ended." << std::endl;
}

void initialize_in_strings(int len) {
  vector<StringPiece> recurring;
  for (int i = 0; i<=len/100 ; i++) {
    SavedString *r = create_random_string(16, true);
    recurring.push_back(r);
  }

  for (int i = 0; i<len; i++) {
    int a = rand()%3;
    if (a ==0) {
      in_strings.push_back(new_saved_string(recurring[rand()%recurring.size()]));
    } else if (a ==1) {
      in_strings.push_back(create_random_string(16));
    } else {
      in_strings.push_back(create_random_string(2));
    }
  }

}

void initialize_out_strings(int len) {
  for (int i = 0; i<len; i++) {
    out_strings.push_back(InternedToken{.value=0});
  }
}

int test_basic() {
  Sharded interner;
  const char *s = "hello_world";
  StringPiece sp(s);

  InternedToken tok = interner.intern(sp);
  cout<<tok<<endl;
  cout<<s<<endl;
  cout<<sp<<endl;

  return 0;
}


int test_major() {
  //int len = 10*1000*1000;
  int len = 10*1000*1000;
  cout<<"Starting to test interning."<<endl;
  cout<<"Generating test data."<<endl;
  initialize_in_strings(len);
  initialize_out_strings(len);
  assert(in_strings.size() == len);
  assert(out_strings.size() == len);

  cout<<"Interning..."<<endl;
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
  cout<<"Interned "<<len<<" strings using "<<tot_threads<<" threads in "<<elapsed_seconds.count()<<"s."<<endl;

  cout<<"Validating"<<endl;
  cout<<"Checking that the interned versions are equal to the original... "<<std::flush;
  for (int i=0; i<len; i++) {
    assert(out_strings[i].value);
    assert (in_strings[i] == out_strings[i]);
  }
  cout<<"Done."<<endl;

  cout<<"Checking that duplicates are properly de-deplicated (this check uses std::string and std::unordered_map, so it might be slow)... "<<std::flush;
  std::unordered_map<std::string, hash_t> m;
  //std::unordered_map<std::string, int> cnt;

  // Used as a sanity test for the test itself. If no duplicate was ever tested, then there is a problem with the test.
  int num_tested_duplicate = 0;

  for (int i=0; i<len; i++) {
    string s(in_strings[i]);
    if (m.count(s)) {
      assert(m[s] == out_strings[i].value);
      num_tested_duplicate+=1;
    } else {
      m[s] = out_strings[i].value;
    }
    //cnt[s]++;
  }
  /*
  for(auto &a: cnt) {
    cout<<a.first<<" "<<a.second<<endl;
  }
  */

  assert(num_tested_duplicate);
  cout<<"Done."<<endl;
  //cout<<(double)num_tested_duplicate/len<<endl;


  // TODO; intrusively test the insides of the interner to see that no other data was created.


  return 0;
}

int main() {

  int ret = 0;

  if(ret = test_basic()) {
    return ret;
  }

  //return 0;
  if(ret = test_major()) {
    return ret;
  }

}
