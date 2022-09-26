; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -disable-output -print-after=hir-vplan-vec  -vplan-force-vf=2 -vplan-enable-general-peeling-hir -march=core-avx2 < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -disable-output -vplan-force-vf=2 -vplan-enable-general-peeling-hir -march=core-avx2 < %s 2>&1 | FileCheck %s

; This issue was discovered during extensive alloy testing of peeling and alignment analysis
; changes. We have a loop with a trip count of 3. Planner does a scalar peel of 1 iteration
; and chooses VF == 2 for the main loop. Note that we do not need a remainder loop in this
; case. Original loop is a constant trip count loop and we were trying to do complete
; unroll of the vector loop. However, due to the way the merged CFG is setup, the lower
; bound of the vector loop is not a constant and this causes a crash in complete unroller.
; The fix checks for no peeling before we do complete unroll.
;
@arr = internal unnamed_addr global [40 x i8] zeroinitializer, align 32

;

; CHECK:           + DO i1 = %ub.tmp, {{.*}}, 2   <DO_LOOP> <auto-vectorized> <nounroll> <novectorize>
; CHECK-NEXT:      |   (<2 x i32>*)(bitcast (i8* getelementptr inbounds ([40 x i8], [40 x i8]* @arr, i64 0, i64 20) to i32*))[i1] = 0;
; CHECK-NEXT:      + END LOOP
;
define void @foo() {
entry:
  br label %for.body

for.body:
  %t709 = phi i64 [ %t711, %for.body ], [ 1, %entry ]
  %t710 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) bitcast (i8* getelementptr inbounds ([40 x i8], [40 x i8]* @arr, i64 0, i64 20) to i32*), i64 %t709) #10
  store i32 0, i32* %t710, align 4
  %t711 = add nuw nsw i64 %t709, 1
  %t712 = icmp eq i64 %t711, 4
  br i1 %t712, label %for.exit, label %for.body

for.exit:
  ret void
}

declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #4
