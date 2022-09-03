; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -disable-output -print-after=hir-vplan-vec -vplan-force-vf=4 < %s 2>&1 | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -disable-output -print-after=hir-vplan-vec -vplan-vec-scenario="m4;v8;v2s1" < %s 2>&1 | FileCheck %s --check-prefix=VECCHECK
; LIT test to check emission of peel/remainder loop tags during vectorization.
;
; CHECK: DO i1 = 0, %peel.ub, 1   <DO_LOOP>  {{.*}} <vector-peel>
; CHECK: DO i1 = %ub.tmp, %loop.ub, 4   <DO_LOOP> <auto-vectorized>
; CHECK: DO i1 = %lb.tmp, 1023, 1   <DO_LOOP> {{.*}} <vector-remainder>
;
; VECCHECK: DO i1 = 0, 2, 4 <DO_LOOP> <vector-peel>
; VECCHECK: DO i1 = 3, %loop.ub, 8   <DO_LOOP> <auto-vectorized>
; VECCHECK: DO i1 = {{.*}}, {{.*}}, 2  <DO_LOOP> {{.*}} <vector-remainder>
; VECCHECK: DO i1 = {{.*}}, 1023, 1  <DO_LOOP> {{.*}} <vector-remainder>
;
@arr = common global [1025 x i64] zeroinitializer, align 32
define void @foo() {
entry:
  br label %for.body

for.body:
  %l1.06 = phi i64 [ %inc, %for.body ], [ 0, %entry ]
  %add = add nuw nsw i64 %l1.06, 1
  %arrayidx = getelementptr inbounds [1025 x i64], [1025 x i64]* @arr, i64 0, i64 %add
  store i64 %l1.06, i64* %arrayidx, align 8
  %inc = add nuw nsw i64 %l1.06, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body

for.end.loopexit:
  br label %for.end

for.end:
  ret void
}
