// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility %s -emit-llvm -o - | FileCheck %s

// CHECK-LABEL: define i32 @ROLc
unsigned ROLc(unsigned word, const int i)
{
   asm ("roll %2,%0"
      :"=r" (word)
      :"0" (word),"I" (i));
   // CHECK: asm "roll $2,$0", "=r,0,I,~{dirflag},~{fpsr},~{flags}"(i32 %{{.+}}, i32 %{{.+}})
   return word;
}

// CHECK-LABEL: define i64 @ROL64c
unsigned long ROL64c(unsigned long word, const int i)
{
   asm("rolq %2,%0"
      :"=r" (word)
      :"0" (word),"J" (i));
   // CHECK: asm "rolq $2,$0", "=r,0,J,~{dirflag},~{fpsr},~{flags}"(i64 %{{.+}}, i32 %{{.+}})
   return word;
}

