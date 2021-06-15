; RUN: opt -VPlanDriver -S < %s  -disable-vplan-codegen -disable-output

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @test1(i32 *%p) {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %header ]

  %gep = getelementptr i32, i32 *%p, i64 %iv
  %gep.opaque = bitcast i32 *%gep to ptr
  ; Used to crash inside DA's isUnitStridePtr called from SVA run inside CM.
  %ld = load i32, ptr %gep.opaque

  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 128
  br i1 %exitcond, label %exit, label %header

exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
