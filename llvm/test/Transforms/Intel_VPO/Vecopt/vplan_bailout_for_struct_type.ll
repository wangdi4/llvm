; Verify that VPlan bails out early for non-supported types like StructType.

; REQUIRES: asserts
; RUN: opt -VPlanDriver -vplan-force-vf=2 -disable-output -debug-only=LoopVectorizationPlanner < %s 2>&1 | FileCheck %s --check-prefix=SIMD
; RUN: opt -hir-ssa-deconstruction -hir-framework -VPlanDriverHIR -debug-only=LoopVectorizationPlanner -disable-output < %s 2>&1 | FileCheck %s --check-prefix=SIMD

; SIMD: LVP: Unsupported type found.
; SIMD: LVP: VPlan is not legal to process, bailing out.

%complex_64bit = type { float, float }

@pR = internal unnamed_addr global [100 x %complex_64bit] zeroinitializer, align 16
@pS = internal unnamed_addr global [100 x %complex_64bit] zeroinitializer, align 16

define void @simd_test() local_unnamed_addr {
alloca:
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()]
  br label %bb5

bb5:                                            ; preds = %bb5, %alloca
  %p0 = phi i32 [ 0, %simd.begin.region ], [ %add21, %bb5 ]
  %int_sext17 = sext i32 %p0 to i64
  %pR = getelementptr inbounds [100 x %complex_64bit], [100 x %complex_64bit]* @pR, i32 0, i64 %int_sext17
  %pR_fetch = load %complex_64bit, %complex_64bit* %pR
  %pS = getelementptr inbounds [100 x %complex_64bit], [100 x %complex_64bit]* @pS, i32 0, i64 %int_sext17
  store %complex_64bit %pR_fetch, %complex_64bit* %pS
  %add21 = add nsw i32 %p0, 1
  %rel = icmp sle i32 %add21, 5
  br i1 %rel, label %bb5, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %bb1

bb1:                                              ; preds = %bb5
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)


; Incoming HIR
;    BEGIN REGION { }
;          %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;
;          + DO i1 = 0, 5, 1   <DO_LOOP>
;          |   %pR_fetch = (@pR)[0][i1];
;          |   (@pS)[0][i1] = %pR_fetch;
;          + END LOOP
;
;          @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
;    END REGION

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -print-after=VPlanDriverHIR -disable-output < %s 2>&1 | FileCheck %s --check-prefix=AUTOVEC

; AUTOVEC-LABEL: Function: autovec_test
; AUTOVEC:           BEGIN REGION { }
; AUTOVEC-NEXT:            + DO i1 = 0, 5, 1   <DO_LOOP>
; AUTOVEC-NEXT:            |   %pR_fetch = (@pR)[0][i1];
; AUTOVEC-NEXT:            |   (@pS)[0][i1] = %pR_fetch;
; AUTOVEC-NEXT:            + END LOOP
; AUTOVEC-NEXT:      END REGION

define void @autovec_test() local_unnamed_addr {
alloca:
  br label %bb5

bb5:                                            ; preds = %bb5, %alloca
  %p0 = phi i32 [ 0, %alloca ], [ %add21, %bb5 ]
  %int_sext17 = sext i32 %p0 to i64
  %pR = getelementptr inbounds [100 x %complex_64bit], [100 x %complex_64bit]* @pR, i32 0, i64 %int_sext17
  %pR_fetch = load %complex_64bit, %complex_64bit* %pR
  %pS = getelementptr inbounds [100 x %complex_64bit], [100 x %complex_64bit]* @pS, i32 0, i64 %int_sext17
  store %complex_64bit %pR_fetch, %complex_64bit* %pS
  %add21 = add nsw i32 %p0, 1
  %rel = icmp sle i32 %add21, 5
  br i1 %rel, label %bb5, label %bb1

bb1:                                              ; preds = %bb5
  ret void
}
