// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s
//***INTEL: pragma inline test

#pragma inline ; // expected-error {{this pragma must immediately precede a statement}}
struct S {
  #pragma inline recursive // expected-error {{this pragma must immediately precede a statement}}
  int a;
} d;

class C {
  int a;
  public:
  #pragma inline // expected-error {{this pragma must immediately precede a statement}}
  int b;
} e;

int main(int argc, char **argv)
{
  #pragma inline // expected-error {{this pragma may not be used here}}
  int i, lll;
  int j, k, aaa;
  i = 1;
  #pragma inline recursive
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
  i++;
  for(j = i; j < argc; ++i)
  ;

inline_label1:  
  #pragma inline
  i+=j;
  #pragma inline recursive
  do
  {
    ++i;
  }
  while (i > argc);
  
  #pragma inline
  while(i > argc)
  ;
  #pragma inline
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
  #pragma inline
    for(k = 0; k < argc; k++)
    ;
  }
  ;
inline_label2:  
  #pragma inline
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
      #pragma inline // expected-error {{this pragma may not be used here}}
      #pragma inline // expected-error {{this pragma may not be used here}}
      #pragma inline // expected-error {{this pragma may not be used here}}
      #pragma inline recursive // expected-error {{this pragma may not be used here}}
      #pragma inline // expected-error {{this pragma may not be used here}}
      #pragma inline // expected-error {{this pragma may not be used here}}
      break;
    case (1):
      #pragma inline
      #pragma inline recursive 
      ;
      break;
    case (2):
      #pragma inline // expected-error {{this pragma may not be used here}}
      break;
    default:
      #pragma inline
      return (1);
      #pragma inline // expected-error {{this pragma may not be used here}}
  }
  ++i;
  #pragma inline 121212 ; // expected-warning {{extra text after expected end of preprocessing directive}}
  #pragma inline;         // expected-warning {{extra text after expected end of preprocessing directive}}
  ;
  #pragma inline
  
  return (i);
  #pragma inline // expected-error {{this pragma may not be used here}}
  #pragma inline // expected-error {{this pragma may not be used here}}
}
#pragma inline // expected-error {{this pragma must immediately precede a statement}}

void www()
{
  #pragma inline
  return;
}

void vvv()
{
  goto label1;
  #pragma inline
label1:
  #pragma inline
} // expected-error {{expected statement}}

