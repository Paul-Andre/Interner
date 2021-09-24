#pragma once

struct SavedString{
  ssize_t size;
  char data[];
};

struct StringPiece{
  const char *data;
  ssize_t size;
  StringPiece(const SavedString *s) {
    data = s->data;
    size = s->size;
  }
  StringPiece(const string &s) {
    size = s->size();
    data = s->c_str();
  }
};
