; RUN: opt -S < %s -passes="vec-clone,alignment-from-assumptions,vplan-vec" \
; RUN:   -vplan-print-after-init -vplan-print-after-align-assume-cleanup \
; RUN:   -debug-only=AlignAssumeCleanup -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: VPlan after initial VPlan
; CHECK:      ptr [[A_GEP:%.*]] = getelementptr ptr, ptr %vec.a i32 {{.*}}
; CHECK-NEXT: ptr [[A_ELEM:%.*]] = load ptr [[A_GEP]]
; CHECK-NEXT: call i1 true ptr [[A_ELEM]] i64 16 ptr @llvm.assume

; CHECK: Removing alignment assumption: {{.*}} call i1 true ptr [[A_ELEM]] i64 16 ptr @llvm.assume

; CHECK-LABEL: VPlan after cleaning up alignment assumptions
; CHECK:      ptr [[A_GEP]] = getelementptr ptr, ptr %vec.a
; CHECK-NEXT: ptr [[A_ELEM]] = load ptr [[A_GEP]]
; CHECK-NOT:  call i1 true ptr [[A_ELEM]] i64 i16 ptr @llvm.assume

define i32 @foo(ptr %a) #0 {
entry:
  %0 = load i32, ptr %a, align 4
  ret i32 %0
}

attributes #0 = { nounwind uwtable "vector-variants"="_ZGVbN4va16_foo" }
