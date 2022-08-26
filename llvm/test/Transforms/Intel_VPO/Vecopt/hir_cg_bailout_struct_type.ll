; Check that VPlan HIR vectorizer bails out for instructions producing non-vectorizable
; types like StructType. This should be fixed after HIR vectorizer CG supports
; serialization.

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

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -disable-output < %s 2>&1 | FileCheck %s


; CHECK-LABEL: Function: autovec_test
; CHECK:           BEGIN REGION { }
; CHECK-NEXT:            + DO i1 = 0, 5, 1   <DO_LOOP>
; CHECK-NEXT:            |   %pR_fetch = (@pR)[0][i1];
; CHECK-NEXT:            |   (@pS)[0][i1] = %pR_fetch;
; CHECK-NEXT:            + END LOOP
; CHECK-NEXT:      END REGION

%complex_64bit = type { float, float }

@pR = internal unnamed_addr global [100 x %complex_64bit] zeroinitializer, align 16
@pS = internal unnamed_addr global [100 x %complex_64bit] zeroinitializer, align 16

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

