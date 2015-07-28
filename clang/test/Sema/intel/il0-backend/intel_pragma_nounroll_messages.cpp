// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s
//***INTEL: pragma nounroll test

#pragma nounroll ; // expected-error {{this pragma must immediately precede a statement}}
struct S {
  #pragma nounroll  // expected-error {{this pragma must immediately precede a statement}}
  int a;
} d;

class C {
  int a;
  public:
  #pragma nounroll // expected-error {{this pragma must immediately precede a statement}}
  int b;
} e;

int main(int argc, char **argv)
{
  #pragma nounroll // expected-error {{this pragma may not be used here}}
  int i, lll;
  int j, k, aaa;
  i = 1;
  #pragma nounroll 
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
  i++;
  for(j = i; j < argc; ++i)
  ;

nounroll_label1:  
  #pragma nounroll
  i+=j;
  #pragma nounroll 
  do
  {
    ++i;
  }
  while (i > argc);
  
  #pragma nounroll
  while(i > argc)
  ;
  #pragma nounroll
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
  #pragma nounroll
    for(k = 0; k < argc; k++)
    ;
  }
  ;
nounroll_label2:  
  #pragma nounroll
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
      #pragma nounroll // expected-error {{this pragma may not be used here}}
      #pragma nounroll // expected-error {{this pragma may not be used here}}
      #pragma nounroll // expected-error {{this pragma may not be used here}}
      #pragma nounroll  // expected-error {{this pragma may not be used here}}
      #pragma nounroll // expected-error {{this pragma may not be used here}}
      #pragma nounroll // expected-error {{this pragma may not be used here}}
      break;
    case (1):
      #pragma nounroll
      #pragma nounroll  
      ;
      break;
    case (2):
      #pragma nounroll // expected-error {{this pragma may not be used here}}
      break;
    default:
      #pragma nounroll
      return (1);
      #pragma nounroll // expected-error {{this pragma may not be used here}}
  }
  ++i;
  #pragma nounroll 121212 ; // expected-warning {{extra text after expected end of preprocessing directive}}
  #pragma nounroll;         // expected-warning {{extra text after expected end of preprocessing directive}}
  ;
  #pragma nounroll
  
  return (i);
  #pragma nounroll // expected-error {{this pragma may not be used here}}
  #pragma nounroll // expected-error {{this pragma may not be used here}}
}
#pragma nounroll // expected-error {{this pragma must immediately precede a statement}}

void www()
{
  #pragma nounroll
  return;
}

void vvv()
{
  goto label1;
  #pragma nounroll
label1:
  #pragma nounroll
} // expected-error {{expected statement}}

