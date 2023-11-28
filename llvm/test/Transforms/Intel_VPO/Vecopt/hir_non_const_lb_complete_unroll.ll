; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -disable-output -vplan-force-vf=2 -vplan-enable-general-peeling-hir -march=core-avx2 < %s 2>&1 | FileCheck %s

; This issue was discovered during extensive alloy testing of peeling and alignment analysis
; changes. We have a loop with a trip count of 3. Planner does a scalar peel of 1 iteration
; and chooses VF == 2 for the main loop. Note that we do not need a remainder loop in this
; case. Original loop is a constant trip count loop and we were trying to do complete
; unroll of the vector loop. However, due to the way the merged CFG is setup, the lower
; bound of the vector loop is not a constant and this causes a crash in complete unroller.
; The fix checks for no peeling before we do complete unroll.
;

; HIR before vectorization:
; BEGIN REGION { }
;       %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;
;       + DO i1 = 0, 2, 1   <DO_LOOP>
;       |   (getelementptr inbounds ([40 x i8], ptr @arr, i64 0, i64 20))[i1] = 0;
;       + END LOOP
;
;       @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; END REGION

@arr = internal unnamed_addr global [40 x i8] zeroinitializer, align 32

;

; CHECK:           + DO i1 = %ub.tmp, {{.*}}, 2   <DO_LOOP>  <MAX_TC_EST = 1> <LEGAL_MAX_TC = 1> <auto-vectorized> <nounroll> <novectorize> <max_trip_count = 1>
; CHECK-NEXT:      |   (<2 x i32>*)(getelementptr inbounds ([40 x i8], ptr @arr, i64 0, i64 20))[i1] = 0;
; CHECK-NEXT:      + END LOOP
;
define void @foo() {
entry:
  br label %for.body

for.body:
  %t709 = phi i64 [ %t711, %for.body ], [ 1, %entry ]
  %t710 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) getelementptr inbounds ([40 x i8], ptr @arr, i64 0, i64 20), i64 %t709) #10
  store i32 0, ptr %t710, align 4
  %t711 = add nuw nsw i64 %t709, 1
  %t712 = icmp eq i64 %t711, 4
  br i1 %t712, label %for.exit, label %for.body

for.exit:
  ret void
}

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #4
