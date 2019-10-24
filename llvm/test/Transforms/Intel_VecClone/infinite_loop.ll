; RUN: opt -vec-clone -S < %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK-LABEL: @_ZGVdN8_foo(
; CHECK-NEXT:    br label [[SIMD_BEGIN_REGION:%.*]]
; CHECK:       simd.begin.region:
; CHECK-NEXT:    [[ENTRY_REGION:%.*]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8) ]
; CHECK-NEXT:    br label [[SIMD_LOOP:%.*]]
; CHECK:       simd.loop:
; CHECK-NEXT:    [[INDEX:%.*]] = phi i32 [ 0, [[SIMD_BEGIN_REGION]] ], [ [[INDVAR:%.*]], [[SIMD_LOOP_EXIT:%.*]] ]
; CHECK-NEXT:    br label [[TMP1:%.*]]
; CHECK:       1:
; CHECK-NEXT:    br label [[TMP1]]
; CHECK:       simd.loop.exit:
; CHECK-NEXT:    [[INDVAR]] = add nuw i32 [[INDEX]], 1
; CHECK-NEXT:    [[VL_COND:%.*]] = icmp ult i32 [[INDVAR]], 8
; CHECK-NEXT:    br i1 [[VL_COND]], label [[SIMD_LOOP]], label [[SIMD_END_REGION:%.*]], !llvm.loop !0
; CHECK:       simd.end.region:
; CHECK-NEXT:    call void @llvm.directive.region.exit(token [[ENTRY_REGION]]) [ "DIR.OMP.END.SIMD"() ]
; CHECK-NEXT:    br label [[UNREACHABLE_RET:%.*]]
; CHECK:       unreachable.ret:
; CHECK-NEXT:    unreachable

define void @_Zfoo() local_unnamed_addr #0  {
  br label %1

1:
  br label %1
}

attributes #0 = { nounwind "vector-variants"="_ZGVdN8__Zfoo" }
