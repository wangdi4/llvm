// RUN: %clang_cc1 -fsyntax-only -fintel-compatibility -verify %s

class CTest
{
public:
    struct CData; // expected-note {{previously declared 'public' here}}
private:
    struct CData { char  wData; }; // expected-warning {{'CData' redeclared with 'private' access}}
};

int main() {
  CTest::CData data;
  return data.wData;
}
