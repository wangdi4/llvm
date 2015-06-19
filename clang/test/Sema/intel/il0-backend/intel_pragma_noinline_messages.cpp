// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s
//***INTEL: pragma noinline test

#pragma noinline ; // expected-error {{this pragma must immediately precede a statement}}
struct S {
  #pragma noinline  // expected-error {{this pragma must immediately precede a statement}}
  int a;
} d;

class C {
  int a;
  public:
  #pragma noinline // expected-error {{this pragma must immediately precede a statement}}
  int b;
} e;

int main(int argc, char **argv)
{
  #pragma noinline // expected-error {{this pragma may not be used here}}
  int i, lll;
  int j, k, aaa;
  i = 1;
  #pragma noinline 
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
  i++;
  for(j = i; j < argc; ++i)
  ;

noinline_label1:  
  #pragma noinline
  i+=j;
  #pragma noinline 
  do
  {
    ++i;
  }
  while (i > argc);
  
  #pragma noinline
  while(i > argc)
  ;
  #pragma noinline
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
  #pragma noinline
    for(k = 0; k < argc; k++)
    ;
  }
  ;
noinline_label2:  
  #pragma noinline
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
      #pragma noinline // expected-error {{this pragma may not be used here}}
      #pragma noinline // expected-error {{this pragma may not be used here}}
      #pragma noinline // expected-error {{this pragma may not be used here}}
      #pragma noinline  // expected-error {{this pragma may not be used here}}
      #pragma noinline // expected-error {{this pragma may not be used here}}
      #pragma noinline // expected-error {{this pragma may not be used here}}
      break;
    case (1):
      #pragma noinline
      #pragma noinline  
      ;
      break;
    case (2):
      #pragma noinline // expected-error {{this pragma may not be used here}}
      break;
    default:
      #pragma noinline
      return (1);
      #pragma noinline // expected-error {{this pragma may not be used here}}
  }
  ++i;
  #pragma noinline 121212 ; // expected-warning {{extra text after expected end of preprocessing directive}}
  #pragma noinline;         // expected-warning {{extra text after expected end of preprocessing directive}}
  ;
  #pragma noinline
  
  return (i);
  #pragma noinline // expected-error {{this pragma may not be used here}}
  #pragma noinline // expected-error {{this pragma may not be used here}}
}
#pragma noinline // expected-error {{this pragma must immediately precede a statement}}

void www()
{
  #pragma noinline
  return;
}

void vvv()
{
  goto label1;
  #pragma noinline
label1:
  #pragma noinline
} // expected-error {{expected statement}}

