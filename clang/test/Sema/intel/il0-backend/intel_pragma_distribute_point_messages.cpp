// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s
//***INTEL: pragma distribute_point test

#pragma distribute_point // expected-error {{this pragma must immediately precede a statement}}
struct S {
  #pragma distribute_point // expected-error {{this pragma must immediately precede a statement}}
  int a;
} d;

class C {
  int a;
  public:
  #pragma distribute_point // expected-error {{this pragma must immediately precede a statement}}
  int b;
} e;

int main(int argc, char **argv)
{
  #pragma distribute_point // expected-error {{this pragma may not be used here}}
  int i, lll;
  int j, k, aaa;
  i = 1;
  #pragma distribute_point
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
  i++;
  for(j = i; j < argc; ++i)
  ;

distribute_point_label1:  
  #pragma distribute_point
  i+=j;
  #pragma distribute_point
  do
  {
    ++i;
  }
  while (i > argc);
  
  #pragma distribute_point
  while(i > argc)
  ;
  #pragma distribute_point
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
  #pragma distribute_point
    for(k = 0; k < argc; k++)
    ;
  }
  ;
distribute_point_label2:  
  #pragma distribute_point
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
      #pragma distribute_point // expected-error {{this pragma may not be used here}}
      #pragma distribute_point // expected-error {{this pragma may not be used here}}
      #pragma distribute_point // expected-error {{this pragma may not be used here}}
      #pragma distribute_point // expected-error {{this pragma may not be used here}}
      #pragma distribute_point // expected-error {{this pragma may not be used here}}
      #pragma distribute_point // expected-error {{this pragma may not be used here}}
      break;
    case (1):
      #pragma distribute_point
      #pragma distribute_point
      ;
      break;
    case (2):
      #pragma distribute_point // expected-error {{this pragma may not be used here}}
      break;
    default:
      #pragma distribute_point
      return (1);
      #pragma distribute_point // expected-error {{this pragma may not be used here}}
  }
  ++i;
  #pragma distribute_point 121212 ; 
  #pragma distribute_point; 
  ;
  #pragma distribute_point
  
  return (i);
  #pragma distribute_point // expected-error {{this pragma may not be used here}}
  #pragma distribute_point // expected-error {{this pragma may not be used here}}
}
#pragma distribute_point // expected-error {{this pragma must immediately precede a statement}}

void www()
{
  #pragma distribute_point
  return;
}

void vvv()
{
  goto label1;
  #pragma distribute_point
label1:
  #pragma distribute_point
} // expected-error {{expected statement}}

