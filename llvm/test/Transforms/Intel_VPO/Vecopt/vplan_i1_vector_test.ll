; Check that we are able to succesfully vectorize the loop. We were
; hitting an incorrect assert which was expecting type size to be a
; a multiple of 8 bits.
; RUN: opt -passes=vplan-vec -S < %s | FileCheck %s
; CHECK-LABEL: vector.body:
define dso_local void @foo2(ptr nocapture %arr, <2 x i1> %l2) local_unnamed_addr #0 {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:
  %.omp.iv.local.010 = phi i64 [ 0, %entry ], [ %add3, %for.body ]
  %ptridx = getelementptr inbounds <2 x i1>, ptr %arr, i64 %.omp.iv.local.010
  %val = load <2 x i1>, ptr %ptridx, align 1
  %add1 = add <2 x i1> %val, %l2
  store <2 x i1> %add1, ptr %ptridx, align 1
  %add3 = add nuw nsw i64 %.omp.iv.local.010, 1
  %exitcond = icmp eq i64 %add3, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
