// RUN: %clang_cc1 -fsyntax-only -verify -pedantic %s -fintel-compatibility -std=c++11
// RUN: %clang_cc1 -fsyntax-only -verify -pedantic %s -fintel-compatibility -std=c++14
char h = '\u1234'; 
char l = 'Ø';  
char m = '👿';  
auto a = 'Ã¼';  //expected-warning {{multi-character character constant}}
   
void foo() {
  switch (a) {
  case 'Ã¼':  //expected-warning {{multi-character character constant}}
    break;
  }
  static_assert(sizeof(a) == 4, "sizeof(a)");
}
