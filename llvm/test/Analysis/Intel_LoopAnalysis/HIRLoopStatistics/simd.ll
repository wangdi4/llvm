; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-loop-statistics>" -disable-output 2>&1 | FileCheck %s

; Verify that we don't have non-SIMD unsafe calls in this loop as the only calls
; encountered are SIMD region.entry and region.exit calls.

; The IR is technically invalid because the SIMD intrinsics are inside the loop
; body. The IR was artificially minimized for testing.

; CHECK: + DO i1 = 0, 1023, 1
; CHECK:   Has unsafe calls: yes
; CHECK:   Has non-SIMD unsafe calls: no
; CHECK: + END LOOP


define void @scalar_soa() {
entry:
  %priv = alloca [1024 x i32], align 4
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* %priv, i64 0, i64 %indvars.iv
  store i32 1, i32* %arrayidx, align 4
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.body, label %omp.loop.exit

omp.loop.exit:
  ret void
}

declare token @llvm.directive.region.entry() #1

declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }
