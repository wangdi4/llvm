// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s
//***INTEL: pragma nounroll_and_jam test

#pragma nounroll_and_jam ; // expected-error {{this pragma must immediately precede a statement}}
struct S {
  #pragma nounroll_and_jam  // expected-error {{this pragma must immediately precede a statement}}
  int a;
} d;

class C {
  int a;
  public:
  #pragma nounroll_and_jam // expected-error {{this pragma must immediately precede a statement}}
  int b;
} e;

int main(int argc, char **argv)
{
  #pragma nounroll_and_jam // expected-error {{this pragma may not be used here}}
  int i, lll;
  int j, k, aaa;
  i = 1;
  #pragma nounroll_and_jam 
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
  i++;
  for(j = i; j < argc; ++i)
  ;

nounroll_and_jam_label1:  
  #pragma nounroll_and_jam
  i+=j;
  #pragma nounroll_and_jam 
  do
  {
    ++i;
  }
  while (i > argc);
  
  #pragma nounroll_and_jam
  while(i > argc)
  ;
  #pragma nounroll_and_jam
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
  #pragma nounroll_and_jam
    for(k = 0; k < argc; k++)
    ;
  }
  ;
nounroll_and_jam_label2:  
  #pragma nounroll_and_jam
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
      #pragma nounroll_and_jam // expected-error {{this pragma may not be used here}}
      #pragma nounroll_and_jam // expected-error {{this pragma may not be used here}}
      #pragma nounroll_and_jam // expected-error {{this pragma may not be used here}}
      #pragma nounroll_and_jam  // expected-error {{this pragma may not be used here}}
      #pragma nounroll_and_jam // expected-error {{this pragma may not be used here}}
      #pragma nounroll_and_jam // expected-error {{this pragma may not be used here}}
      break;
    case (1):
      #pragma nounroll_and_jam
      #pragma nounroll_and_jam  
      ;
      break;
    case (2):
      #pragma nounroll_and_jam // expected-error {{this pragma may not be used here}}
      break;
    default:
      #pragma nounroll_and_jam
      return (1);
      #pragma nounroll_and_jam // expected-error {{this pragma may not be used here}}
  }
  ++i;
  #pragma nounroll_and_jam 121212 ; // expected-warning {{extra text after expected end of preprocessing directive}}
  #pragma nounroll_and_jam;         // expected-warning {{extra text after expected end of preprocessing directive}}
  ;
  #pragma nounroll_and_jam
  
  return (i);
  #pragma nounroll_and_jam // expected-error {{this pragma may not be used here}}
  #pragma nounroll_and_jam // expected-error {{this pragma may not be used here}}
}
#pragma nounroll_and_jam // expected-error {{this pragma must immediately precede a statement}}

void www()
{
  #pragma nounroll_and_jam
  return;
}

void vvv()
{
  goto label1;
  #pragma nounroll_and_jam
label1:
  #pragma nounroll_and_jam
} // expected-error {{expected statement}}

