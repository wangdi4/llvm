; REQUIRES: asserts
; RUN: opt < %s -passes="print<hir-region-identification>" -debug-only=hir-region-identification  2>&1 | FileCheck %s

; Verify that we don't form a region for simd loop if pre loop bblocks contain
; an unsupported instruction like a volatile store.

; CHECK: Bailing out on instruction:
; CHECK: store volatile i32 0, i32* %gep0, align 4

; CHECK: Loop %for.body: Volatile instructions are currently not supported.


define void @scalar_soa() {
entry:
  %priv = alloca [1024 x i32], align 4
  br label %SIMD.BEGIN

SIMD.BEGIN:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2), "QUAL.OMP.PRIVATE"([1024 x i32]* %priv) ]
  %gep0 = getelementptr inbounds [1024 x i32], [1024 x i32]* %priv, i64 0, i64 0
  store volatile i32 0, i32* %gep0, align 4
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %SIMD.BEGIN ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* %priv, i64 0, i64 %indvars.iv
  store i32 1, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.body, label %omp.loop.exit

omp.loop.exit:
  br label %SIMD.END

SIMD.END:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %EXIT

EXIT:
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)
