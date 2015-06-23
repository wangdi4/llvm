// RUN: %clang_cc1 -fintel-compatibility -ferror-limit 100 -emit-llvm -verify -o - %s
//***INTEL: pragma loop_count test

#pragma loop_count ; // expected-error {{this pragma must immediately precede a statement}}
struct S {
  #pragma loop_count=1 // expected-error {{this pragma must immediately precede a statement}}
  int a;
} d;

#define XXX 14

class C {
  int a;
  public:
  #pragma loop_count (4+5) // expected-error {{this pragma must immediately precede a statement}}
  int b;
} e;

int main(int argc, char **argv)
{
  #pragma loop_count =2,3,7*5 // expected-error {{this pragma may not be used here}}
  int i, lll;
  int j, k, aaa;
  i = 1;
  #pragma loop_count min=2 max(10) avg 20
  #pragma loop_count=1
  for(i = 0; i < argc; ++i)
  {
    for(j = i; j < argc; ++i)
    ;
  }
  i++;
  for(j = i; j < argc; ++i)
  ;

loop_count_label1:  
  #pragma loop_count(10)
  i+=j;
  #pragma loop_count=2,4-5,3+2*8 // expected-error {{invalid loop count}}
  do
  {
    ++i;
  }
  while (i > argc);
  
  #pragma loop_count(2,4-5,3+2*8) // expected-error {{invalid loop count}}
  while(i > argc)
  ;
  #pragma loop_count max(10+1),min(8*2), avg=2)
  #pragma loop_count max(10+1),min(8*2), avg(2),min(10) // expected-error {{invalid loop count}}
  #pragma loop_count max(10+1),min(8*2), avg(2),max(11) // expected-error {{invalid loop count}}
  for(int l = i; l < argc; ++l)
  {
    aaa+=lll;
  #pragma loop_count (2
    for(k = 0; k < argc; k++)
    ;
  }
  ;
loop_count_label2:  
  #pragma loop_count max(3), min(2)
  #pragma loop_count 2
  do
  {
  ;
  }
  while (i > argc);
  switch (argc) {
    case (0):
      #pragma loop_count min=1// expected-error {{this pragma may not be used here}}
      #pragma loop_count max=3// expected-error {{this pragma may not be used here}}
      #pragma loop_count avg=2// expected-error {{this pragma may not be used here}}
      #pragma loop_count =2 // expected-error {{this pragma may not be used here}}
      #pragma loop_count (3)// expected-error {{this pragma may not be used here}}
      #pragma loop_count // expected-error {{this pragma may not be used here}}
      break;
    case (1):
      #pragma loop_count min=2,max=5
      #pragma loop_count -2 // expected-error {{invalid loop count}}
      ;
      break;
    case (2):
      #pragma loop_count (12 // expected-error {{this pragma may not be used here}}
      break;
    default:
      #pragma loop_count min=2 max(3) 10 // expected-warning {{extra text after expected end of preprocessing directive}}
      #pragma loop_count 2 max(3) // expected-error {{use of undeclared identifier 'max'}}
      #pragma loop_count XXX
      return (1);
      #pragma loop_count // expected-error {{this pragma may not be used here}}
  }
  ++i;
  #pragma loop_count 121212 
  #pragma loop_count =2323 # 12         // expected-error {{expected expression}}
  ;
  #pragma loop_count =1212121121212
  
  return (i);
  #pragma loop_count // expected-error {{this pragma may not be used here}}
  #pragma loop_count // expected-error {{this pragma may not be used here}}
}
#pragma loop_count // expected-error {{this pragma must immediately precede a statement}}

void www()
{
  #pragma loop_count 12122=334 // expected-error {{expected expression}}
  return;
}

void vvv()
{
  goto label1;
  #pragma loop_count
label1:
  #pragma loop_count
} // expected-error {{expected statement}}

