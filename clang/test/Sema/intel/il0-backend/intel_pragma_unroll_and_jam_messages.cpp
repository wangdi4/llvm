// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s
//***INTEL: pragma unroll_and_jam test

#pragma unroll_and_jam (12; // expected-error {{this pragma must immediately precede a statement}}
struct S {
  #pragma unroll_and_jam (23+4 // expected-error {{this pragma must immediately precede a statement}}
  int a;
} d;

class C {
  int a;
  public:
  #pragma unroll_and_jam // expected-error {{this pragma must immediately precede a statement}}
  int b;
} e;

int main(int argc, char **argv)
{
  #pragma unroll_and_jam // expected-error {{this pragma may not be used here}}
  int i, lll;
  int j, k, aaa;
  i = 1;
  #pragma unroll_and_jam 
  #pragma unroll_and_jam 280 // expected-warning {{missing '(' after '#pragma unroll_and_jam' - ignoring}}
  #pragma unroll_and_jam (280
  #pragma unroll_and_jam (-1
  #pragma unroll_and_jam (14
  #pragma unroll_and_jam (14
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
  i++;
  for(j = i; j < argc; ++i)
  ;

unroll_and_jam_label1:  
  #pragma unroll_and_jam (12*4
  i+=j;
  #pragma unroll_and_jam (7
  do
  {
    ++i;
  }
  while (i > argc);
  
  #pragma unroll_and_jam
  while(i > argc)
  ;
  #pragma unroll_and_jam (0
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
  #pragma unroll_and_jam
    for(k = 0; k < argc; k++)
    ;
  }
  ;
unroll_and_jam_label2:  
  #pragma unroll_and_jam
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
      #pragma unroll_and_jam // expected-error {{this pragma may not be used here}}
      #pragma unroll_and_jam // expected-error {{this pragma may not be used here}}
      #pragma unroll_and_jam // expected-error {{this pragma may not be used here}}
      #pragma unroll_and_jam  // expected-error {{this pragma may not be used here}}
      #pragma unroll_and_jam // expected-error {{this pragma may not be used here}}
      #pragma unroll_and_jam // expected-error {{this pragma may not be used here}}
      break;
    case (1):
      #pragma unroll_and_jam
      #pragma unroll_and_jam (4+9
      #pragma unroll_and_jam (4-8
      ;
      break;
    case (2):
      #pragma unroll_and_jam // expected-error {{this pragma may not be used here}}
      break;
    default:
      #pragma unroll_and_jam
      return (1);
      #pragma unroll_and_jam // expected-error {{this pragma may not be used here}}
  }
  ++i;
  #pragma unroll_and_jam (121212 ; // expected-warning {{extra text after expected end of preprocessing directive}}
  #pragma unroll_and_jam;    // expected-warning {{missing '(' after '#pragma unroll_and_jam' - ignoring}}
  ;
  #pragma unroll_and_jam
  
  return (i);
  #pragma unroll_and_jam // expected-error {{this pragma may not be used here}}
  #pragma unroll_and_jam // expected-error {{this pragma may not be used here}}
}
#pragma unroll_and_jam // expected-error {{this pragma must immediately precede a statement}}

void www()
{
  #pragma unroll_and_jam
  return;
}

void vvv()
{
  goto label1;
  #pragma unroll_and_jam
label1:
  #pragma unroll_and_jam
} // expected-error {{expected statement}}

