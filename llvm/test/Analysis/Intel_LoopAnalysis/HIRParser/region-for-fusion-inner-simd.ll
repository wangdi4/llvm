; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Verify that we are able to form region for fusion for the two sibling loops
; successfully.
; The test for compfailing as SSA deconstruction incorrectly treated it as
; simd region and tried to split off the exit block.

; CHECK: + DO i1 = 0, 31, 1   <DO_LOOP>
; CHECK: |   %i52 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
; CHECK: |
; CHECK: |   + DO i2 = 0, 31, 1   <DO_LOOP>
; CHECK: |   |   %i66 = (%arg3)[%i19 * i1 + i2 + ((1 + %i45) * %i19) + %i46 + 1];
; CHECK: |   |   (%i)[0][32 * i1 + i2] = %i66;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   @llvm.directive.region.exit(%i52); [ DIR.OMP.END.SIMD() ]
; CHECK: + END LOOP


; CHECK: + DO i1 = 0, 31, 1   <DO_LOOP>
; CHECK: + END LOOP


define void @foo(ptr %i, ptr %arg3, i32 %i19, i32 %i45, i32 %i46) {
entry:
  br label %bb50

bb50:                                             ; preds = %bb70, %entry
  %i51 = phi i64 [ 0, %entry ], [ %i53, %bb70 ]
  %i52 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null)]
  %i53 = add nuw nsw i64 %i51, 1
  %i54 = trunc i64 %i53 to i32
  %i55 = add i32 %i45, %i54
  %i56 = mul nsw i32 %i55, %i19
  %i57 = shl nsw i64 %i51, 5
  br label %bb58

bb58:                                             ; preds = %bb58, %bb50
  %i59 = phi i64 [ 0, %bb50 ], [ %i60, %bb58 ]
  %i60 = add nuw nsw i64 %i59, 1
  %i61 = trunc i64 %i60 to i32
  %i62 = add i32 %i46, %i61
  %i63 = add i32 %i62, %i56
  %i64 = sext i32 %i63 to i64
  %i65 = getelementptr inbounds i32, ptr %arg3, i64 %i64
  %i66 = load i32, ptr %i65, align 4
  %i67 = add nuw nsw i64 %i59, %i57
  %i68 = getelementptr inbounds [1024 x i32], ptr %i, i64 0, i64 %i67
  store i32 %i66, ptr %i68, align 4
  %i69 = icmp eq i64 %i60, 32
  br i1 %i69, label %bb70, label %bb58

bb70:                                             ; preds = %bb58
  call void @llvm.directive.region.exit(token %i52) [ "DIR.OMP.END.SIMD"() ]
  %i71 = icmp eq i64 %i53, 32
  br i1 %i71, label %bb47, label %bb50

bb47:                                             ; preds = %bb70
  br label %bb72

bb72:                                             ; preds = %bb72, %bb47
  %i73 = phi i64 [ 0, %bb47 ], [ %i89, %bb72 ]
  %i89 = add nuw nsw i64 %i73, 1
  %i90 = icmp eq i64 %i89, 32
  br i1 %i90, label %bb91, label %bb72

bb91:                                             ; preds = %bb72
  ret void
}

declare token @llvm.directive.region.entry() #3

declare void @llvm.directive.region.exit(token) #3

attributes #3 = { nounwind }

