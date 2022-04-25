; This test fails when the new pass manager is enabled by default.
; CMPLRLLVM-33740
; XFAIL: new_pm_default

; RUN: opt -S -O2 < %s | FileCheck %s --check-prefix=DEFAULT
; RUN: opt -S -O2 -sycl-opt < %s | FileCheck %s --check-prefix=SYCLOPT
;
; This test is based on the following C code. If sycl-opt is disabled, the
; optimizer should jump thread through the block containing the call to f2
; rather than convert the conditional branches to selects.
; 
; int f1(int v)
; {
;   int state = 0;
;   if ((v & 1) != 0) {
;     v ^= 42;
;     state = 1;
;   }
;   f2(v);
;   if (state) {
;     v *= 42;
;   }
;   return v;
; }
;
; DEFAULT-LABEL: f1
; DEFAULT-NOT: select
; DEFAULT: call void @f2
; DEFAULT-NOT: select
; DEFAULT: call void @f2
; DEFAULT-NOT: select
;
; SYCLOPT-LABEL: f1
; SYCLOPT: select
; SYCLOPT: call void @f2
; SYCLOPT: select

target triple = "x86_64-unknown-linux-gnu"

define i32 @f1(i32 %v) nounwind {
entry:
  %v.addr = alloca i32, align 4
  %state = alloca i32, align 4
  store i32 %v, i32* %v.addr, align 4
  store i32 0, i32* %state, align 4
  %0 = load i32, i32* %v.addr, align 4
  %and = and i32 %0, 1
  %cmp = icmp ne i32 %and, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %1 = load i32, i32* %v.addr, align 4
  %xor = xor i32 %1, 42
  store i32 %xor, i32* %v.addr, align 4
  store i32 1, i32* %state, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %2 = load i32, i32* %v.addr, align 4
  call void @f2(i32 %2)
  %3 = load i32, i32* %state, align 4
  %tobool = icmp ne i32 %3, 0
  br i1 %tobool, label %if.then.1, label %if.end.2

if.then.1:                                        ; preds = %if.end
  %4 = load i32, i32* %v.addr, align 4
  %mul = mul nsw i32 %4, 42
  store i32 %mul, i32* %v.addr, align 4
  br label %if.end.2

if.end.2:                                         ; preds = %if.then.1, %if.end
  %5 = load i32, i32* %v.addr, align 4
  ret i32 %5
}

declare void @f2(i32) nounwind
