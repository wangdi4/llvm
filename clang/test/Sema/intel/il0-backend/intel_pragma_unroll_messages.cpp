// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s
//***INTEL: pragma unroll test

#pragma unroll (12; // expected-error {{this pragma must immediately precede a statement}}
struct S {
  #pragma unroll (23+4 // expected-error {{this pragma must immediately precede a statement}}
  int a;
} d;

class C {
  int a;
  public:
  #pragma unroll // expected-error {{this pragma must immediately precede a statement}}
  int b;
} e;

int main(int argc, char **argv)
{
  #pragma unroll // expected-error {{this pragma may not be used here}}
  int i, lll;
  int j, k, aaa;
  i = 1;
  #pragma unroll 
  #pragma unroll 280 // expected-warning {{missing '(' after '#pragma unroll' - ignoring}}
  #pragma unroll (280
  #pragma unroll (-1
  #pragma unroll (14
  #pragma unroll (14
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
  i++;
  for(j = i; j < argc; ++i)
  ;

unroll_label1:  
  #pragma unroll (12*4
  i+=j;
  #pragma unroll (7
  do
  {
    ++i;
  }
  while (i > argc);
  
  #pragma unroll
  while(i > argc)
  ;
  #pragma unroll (0
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
  #pragma unroll
    for(k = 0; k < argc; k++)
    ;
  }
  ;
unroll_label2:  
  #pragma unroll
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
      #pragma unroll // expected-error {{this pragma may not be used here}}
      #pragma unroll // expected-error {{this pragma may not be used here}}
      #pragma unroll // expected-error {{this pragma may not be used here}}
      #pragma unroll  // expected-error {{this pragma may not be used here}}
      #pragma unroll // expected-error {{this pragma may not be used here}}
      #pragma unroll // expected-error {{this pragma may not be used here}}
      break;
    case (1):
      #pragma unroll
      #pragma unroll (4+9
      #pragma unroll (4-8
      ;
      break;
    case (2):
      #pragma unroll // expected-error {{this pragma may not be used here}}
      break;
    default:
      #pragma unroll
      return (1);
      #pragma unroll // expected-error {{this pragma may not be used here}}
  }
  ++i;
  #pragma unroll (121212 ; // expected-warning {{extra text after expected end of preprocessing directive}}
  #pragma unroll;    // expected-warning {{missing '(' after '#pragma unroll' - ignoring}}
  ;
  #pragma unroll
  
  return (i);
  #pragma unroll // expected-error {{this pragma may not be used here}}
  #pragma unroll // expected-error {{this pragma may not be used here}}
}
#pragma unroll // expected-error {{this pragma must immediately precede a statement}}

void www()
{
  #pragma unroll
  return;
}

void vvv()
{
  goto label1;
  #pragma unroll
label1:
  #pragma unroll
} // expected-error {{expected statement}}

