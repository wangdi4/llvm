// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s
//***INTEL: pragma forceinline test

#pragma forceinline ; // expected-error {{this pragma must immediately precede a statement}}
struct S {
  #pragma forceinline recursive // expected-error {{this pragma must immediately precede a statement}}
  int a;
} d;

class C {
  int a;
  public:
  #pragma forceinline // expected-error {{this pragma must immediately precede a statement}}
  int b;
} e;

int main(int argc, char **argv)
{
  #pragma forceinline // expected-error {{this pragma may not be used here}}
  int i, lll;
  int j, k, aaa;
  i = 1;
  #pragma forceinline recursive
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
  i++;
  for(j = i; j < argc; ++i)
  ;

forceinline_label1:  
  #pragma forceinline
  i+=j;
  #pragma forceinline recursive
  do
  {
    ++i;
  }
  while (i > argc);
  
  #pragma forceinline
  while(i > argc)
  ;
  #pragma forceinline
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
  #pragma forceinline
    for(k = 0; k < argc; k++)
    ;
  }
  ;
forceinline_label2:  
  #pragma forceinline
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
      #pragma forceinline // expected-error {{this pragma may not be used here}}
      #pragma forceinline // expected-error {{this pragma may not be used here}}
      #pragma forceinline // expected-error {{this pragma may not be used here}}
      #pragma forceinline recursive // expected-error {{this pragma may not be used here}}
      #pragma forceinline // expected-error {{this pragma may not be used here}}
      #pragma forceinline // expected-error {{this pragma may not be used here}}
      break;
    case (1):
      #pragma forceinline
      #pragma forceinline recursive 
      ;
      break;
    case (2):
      #pragma forceinline // expected-error {{this pragma may not be used here}}
      break;
    default:
      #pragma forceinline
      return (1);
      #pragma forceinline // expected-error {{this pragma may not be used here}}
  }
  ++i;
  #pragma forceinline 121212 ; // expected-warning {{extra text after expected end of preprocessing directive}}
  #pragma forceinline;         // expected-warning {{extra text after expected end of preprocessing directive}}
  ;
  #pragma forceinline
  
  return (i);
  #pragma forceinline // expected-error {{this pragma may not be used here}}
  #pragma forceinline // expected-error {{this pragma may not be used here}}
}
#pragma forceinline // expected-error {{this pragma must immediately precede a statement}}

void www()
{
  #pragma forceinline
  return;
}

void vvv()
{
  goto label1;
  #pragma forceinline
label1:
  #pragma forceinline
} // expected-error {{expected statement}}

