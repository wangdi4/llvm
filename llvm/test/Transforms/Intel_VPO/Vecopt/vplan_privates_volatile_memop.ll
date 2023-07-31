; This test checks that we mark private variable with volatile stores as unsafe.

; RUN: opt -disable-output -passes=vplan-vec -vplan-enable-soa -vplan-dump-soa-info -disable-vplan-codegen %s 2>&1 | FileCheck %s

; REQUIRES:asserts

; CHECK:  SOAUnsafe = [[VP_PRIV:%.*]] (priv)
define void @volatile_store_privates() {
entry:
  %priv = alloca [1024 x i32], align 4
  br label %SIMD.BEGIN

SIMD.BEGIN:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2), "QUAL.OMP.PRIVATE:TYPED"(ptr %priv, i32 0, i32 1024) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %SIMD.BEGIN ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], ptr %priv, i64 0, i64 %indvars.iv
  store volatile i32 1, ptr %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.body, label %omp.loop.exit

omp.loop.exit:
  br label %SIMD.END

SIMD.END:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
