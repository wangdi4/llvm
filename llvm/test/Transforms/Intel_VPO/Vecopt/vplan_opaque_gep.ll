; RUN: opt -passes=vplan-vec -disable-output -vplan-print-after-plain-cfg < %s | FileCheck %s -check-prefix=VPLAN
; RUN: opt -passes=vplan-vec -S < %s | FileCheck %s -check-prefix=LLVM

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @test1(ptr %p) {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %header ]


  %gep = getelementptr i32, ptr %p, i64 %iv
; VPLAN:  ptr [[VP_GEP:%.*]] = getelementptr i32, ptr [[VP_P:%.*]] i64 [[VP_IV:%.*]]

; LLVM:         [[BROADCAST_SPLATINSERT:%.*]] = insertelement <4 x ptr> poison, ptr [[TMP0:%.*]], i64 0
; LLVM-NEXT:    [[BROADCAST_SPLAT:%.*]] = shufflevector <4 x ptr> [[BROADCAST_SPLATINSERT]], <4 x ptr> poison, <4 x i32> zeroinitializer
; LLVM:         [[MM_VECTORGEP:%.*]] = getelementptr i32, <4 x ptr> [[BROADCAST_SPLAT]], <4 x i64> [[VEC_PHI:%.*]]

  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 128
  br i1 %exitcond, label %exit, label %header

exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
