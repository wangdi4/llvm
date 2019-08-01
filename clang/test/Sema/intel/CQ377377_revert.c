// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility %s -verify -emit-llvm -o - 

// CHECK-LABEL: define i32 @ROLc
unsigned ROLc(unsigned word, const int i)
{
   asm ("roll %2,%0"
      :"=r" (word)
      :"0" (word),"I" (i));
   //expected-error@-1{{constraint 'I' expects an integer constant expression}}
   return word;
}

// CHECK-LABEL: define i64 @ROL64c
unsigned long ROL64c(unsigned long word, const int i)
{
   asm("rolq %2,%0"
      :"=r" (word)
      :"0" (word),"J" (i));
   //expected-error@-1{{constraint 'J' expects an integer constant expression}}
   return word;
}

