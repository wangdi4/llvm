// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s
//***INTEL: pragma nofusion test

#pragma nofusion ; // expected-error {{this pragma must immediately precede a statement}}
struct S {
  #pragma nofusion  // expected-error {{this pragma must immediately precede a statement}}
  int a;
} d;

class C {
  int a;
  public:
  #pragma nofusion // expected-error {{this pragma must immediately precede a statement}}
  int b;
} e;

int main(int argc, char **argv)
{
  #pragma nofusion // expected-error {{this pragma may not be used here}}
  int i, lll;
  int j, k, aaa;
  i = 1;
  #pragma nofusion 
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
  i++;
  for(j = i; j < argc; ++i)
  ;

nofusion_label1:  
  #pragma nofusion
  i+=j;
  #pragma nofusion 
  do
  {
    ++i;
  }
  while (i > argc);
  
  #pragma nofusion
  while(i > argc)
  ;
  #pragma nofusion
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
  #pragma nofusion
    for(k = 0; k < argc; k++)
    ;
  }
  ;
nofusion_label2:  
  #pragma nofusion
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
      #pragma nofusion // expected-error {{this pragma may not be used here}}
      #pragma nofusion // expected-error {{this pragma may not be used here}}
      #pragma nofusion // expected-error {{this pragma may not be used here}}
      #pragma nofusion  // expected-error {{this pragma may not be used here}}
      #pragma nofusion // expected-error {{this pragma may not be used here}}
      #pragma nofusion // expected-error {{this pragma may not be used here}}
      break;
    case (1):
      #pragma nofusion
      #pragma nofusion  
      ;
      break;
    case (2):
      #pragma nofusion // expected-error {{this pragma may not be used here}}
      break;
    default:
      #pragma nofusion
      return (1);
      #pragma nofusion // expected-error {{this pragma may not be used here}}
  }
  ++i;
  #pragma nofusion 121212 ; 
  #pragma nofusion;         
  ;
  #pragma nofusion
  
  return (i);
  #pragma nofusion // expected-error {{this pragma may not be used here}}
  #pragma nofusion // expected-error {{this pragma may not be used here}}
}
#pragma nofusion // expected-error {{this pragma must immediately precede a statement}}

void www()
{
  #pragma nofusion
  return;
}

void vvv()
{
  goto label1;
  #pragma nofusion
label1:
  #pragma nofusion
} // expected-error {{expected statement}}

