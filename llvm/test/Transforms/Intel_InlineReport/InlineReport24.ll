; RUN: opt -argpromotion -inline -inline-report=7 -disable-output < %s -S 2>&1 | FileCheck %s
; RUN: opt -passes='argpromotion,cgscc(inline)' -inline-report=7 -disable-output < %s -S 2>&1 | FileCheck %s

; This test does argument promotion and then inlining and dead static
; function elimination to prove that a function can be replaced with
; another, inlined, and deleted without dying while the inlining report
; is generated.

target datalayout = "E-p:64:64:64-a0:0:8-f32:32:32-f64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-v64:64:64-v128:128:128"

%struct.ss = type { i32, i64 }

define internal void @f(%struct.ss* byval(%struct.ss)  %b) nounwind  {
entry:
  %tmp = getelementptr %struct.ss, %struct.ss* %b, i32 0, i32 0
  %tmp1 = load i32, i32* %tmp, align 4
  %tmp2 = add i32 %tmp1, 1
  store i32 %tmp2, i32* %tmp, align 4
  ret void
}

define internal void @g(%struct.ss* byval(%struct.ss) align 32 %b) nounwind {
entry:
  %tmp = getelementptr %struct.ss, %struct.ss* %b, i32 0, i32 0
  %tmp1 = load i32, i32* %tmp, align 4
  %tmp2 = add i32 %tmp1, 1
  store i32 %tmp2, i32* %tmp, align 4
  ret void
}

define i32 @main() nounwind  {
entry:
  %S = alloca %struct.ss
  %tmp1 = getelementptr %struct.ss, %struct.ss* %S, i32 0, i32 0
  store i32 1, i32* %tmp1, align 8
  %tmp4 = getelementptr %struct.ss, %struct.ss* %S, i32 0, i32 1
  store i64 2, i64* %tmp4, align 4
  call void @f(%struct.ss* byval(%struct.ss) %S) nounwind
  call void @g(%struct.ss* byval(%struct.ss) %S) nounwind
  ret i32 0
}

; CHECK-LABEL: DEAD STATIC FUNC: f
; CHECK-LABEL: DEAD STATIC FUNC: g
; CHECK-LABEL: COMPILE FUNC: main
; CHECK: INLINE: f {{.*}}Callee has single callsite and local linkage{{.*}}
; CHECK: INLINE: g {{.*}}Callee has single callsite and local linkage{{.*}}

