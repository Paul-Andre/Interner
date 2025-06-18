//#pragma once
#include <bits/stdc++.h>

using namespace std;

#include "basics.h"
#include "StringPiece.h"
#include "Interner.h"
#include "memory.h"

string randomString() {
  string ret = "";
  int l = 3 + rand()%3;
  for (int i=0; i<l; i++) {
    string choices = "ab";
    char pick = choices[rand()%(choices.size())];
    ret+=pick;
  }
  return ret;
}


struct ResultAndRest {
  Value result;
  StringPiece rest;
};


int next(StringPiece s) {
  if (s.size > 0) {
    return s.data[0];
  }
  return EOF;
}

StringPiece advance(StringPiece s) {
    s.data++;
    s.size--;
    return s;
}


bool isWhitespace(int c) {
  return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

StringPiece skipWhitespace(StringPiece s) {

  while (isWhitespace(next(s))) {
    s = advance(s);
  }
  return s;
}

struct Parser{
  Memory *memory;
  StringInterner *interner;
  vector<Value*> anchors;
  ResultAndRest parseList(StringPiece s);
  ResultAndRest parse(StringPiece s);
};

ResultAndRest Parser::parseList(StringPiece s) {
  s = skipWhitespace(s);

  int c = next(s);
  if (c == EOF || c == ')') {
    return {{tags::EMPTY_LIST, 0}, s};
  }
  ResultAndRest r = parse(s);
  anchors.push_back(&r.result);
  ResultAndRest t = parseList(r.rest);
  anchors.push_back(&t.result);
  Value pair = memory->makePair(r.result, t.result, anchors);
  anchors.pop_back();
  anchors.pop_back();
  return {pair, t.rest};
}
ResultAndRest Parser::parse(StringPiece s) {
  s = skipWhitespace(s);
  int c = next(s);
  assert(c != EOF);
  if (c == '(') {
    s = advance(s);
    ResultAndRest r = parseList(s);
    s = r.rest;
    s = skipWhitespace(s);

    c = next(s);
    if (c == ')'){
      s = advance(s);
    } else {
      // should be an error
    }
    return {r.result, s};
  } else {
    StringPiece p = s;
    int i = 0;
    while (true) {
      int c = next(s);
      if (isWhitespace(c) || c==')' || c=='(' || c==EOF) {
        break;
      } else {
        i++;
        s = advance(s);
      }
    }
    StringPiece identString = {(ssize_t)i, p.data};
    InternedToken tok = interner->intern(identString);
    Value r = {tags::SYMBOL, (uint64_t) tok.value};
    return {r, s};
  }
}

string input = "(hello (good world asdf asdf() adf() (asdfasdf) asdf))";

/*
printList(ostream& ioOut, Value v) {
  if(v.type == EMPTY_LIST) {
    ioOut << ")";
  } else if v.type


}
*/

ostream &operator<<(ostream& ioOut, Value v) {
  if (v.type == tags::NONE) {
    ioOut << "#none";
  } else if (v.type == tags::NONE) {
    ioOut << "#none";
  } else if (v.type == tags::BOOL && v.value==0) {
    ioOut << "#false";
  } else if (v.type == tags::BOOL && v.value!=0) {
    ioOut << "#true";
  } else if (v.type == tags::SYMBOL) {
    InternedToken t;
    t.value = (hash_t) v.value;
    ioOut << t;
  } else if (v.type == tags::FLOAT) {
    double d;
    d = (double) v.value;
    ioOut << d;
  } else if (v.type == tags::PAIR) {
    Value car = v.car();
    Value cdr = v.cdr();
    ioOut << "( " << car << " . " << cdr << " )";
  } else if (v.type == tags::EMPTY_LIST) {
    ioOut << "()";
  } else {
    ioOut << "#unknown";
  }
  return ioOut;
}

int main() {
  StringInterner interner;
  Memory memory;
  Parser p{&memory, &interner, {}};

  ResultAndRest r = p.parse(input);
  memory.print();
  interner.print();
  cout << r.result <<endl;
  cout << input <<endl;
}

