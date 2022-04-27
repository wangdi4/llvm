; RUN: opt < %s -analyze -enable-new-pm=0 -hir-ssa-deconstruction -hir-framework 2>&1  | FileCheck %s --check-prefix=CHECK-HIR
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" 2>&1  | FileCheck %s --check-prefix=CHECK-HIR

; Check that insertvalue and extractvalue instructions are parsed and printed correctly.

; CHECK-HIR:   + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK-HIR:   |   %pR_fetch = (@pR)[0][i1 + 1];
; CHECK-HIR:   |   %int_cast = sitofp.i32.float(i1 + 1);
; CHECK-HIR:   |   %pR_fetch_c0 = extractvalue %pR_fetch, 0;
; CHECK-HIR:   |   %add = %pR_fetch_c0  +  %int_cast;
; CHECK-HIR:   |   %pR_fetch_c1 = extractvalue %pR_fetch, 1;
; CHECK-HIR:   |   %add6 = %pR_fetch_c1  +  0.000000e+00;
; CHECK-HIR:   |   %insertval8 = insertvalue zeroinitializer,  %add, 0;
; CHECK-HIR:   |   %insertval10 = insertvalue %insertval8,  %add6, 1;
; CHECK-HIR:   |   (@pR)[0][i1 + 1] = %insertval10;
; CHECK-HIR:   + END LOOP

; RUN: opt < %s -hir-ssa-deconstruction -hir-cg -force-hir-cg -S 2>&1 | FileCheck %s --check-prefix=CHECK-CG
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-cg" -force-hir-cg -S 2>&1 | FileCheck %s --check-prefix=CHECK-CG

; CHECK-CG: region.0:
; CHECK-CG:   %pR_fetch_c{{[0-9]+}} = extractvalue %complex_64bit %t{{[.0-9]*}}, 0
; CHECK-CG:   %pR_fetch_c{{[0-9]+}} = extractvalue %complex_64bit %t{{[.0-9]*}}, 1
; CHECK-CG:   %insertval{{[0-9]*}} = insertvalue %complex_64bit zeroinitializer, float %t{{[.0-9]*}}, 0
; CHECK-CG:   %insertval{{[0-9]+}} = insertvalue %complex_64bit %t{{[.0-9]*}}, float %t{{[.0-9]*}}, 1


%complex_64bit = type { float, float }

@pR = internal unnamed_addr global [100 x %complex_64bit] zeroinitializer, align 16

define void @MAIN__() local_unnamed_addr {
alloca:
  br label %bb5

bb5:                                            ; preds = %bb5, %alloca
  %p0 = phi i32 [ 1, %alloca ], [ %add21, %bb5 ]
  %int_sext17 = sext i32 %p0 to i64
  %pR = call %complex_64bit* @llvm.intel.subscript.p0s_complex_64bits.i64.i64.p0s_complex_64bits.i64(i8 0, i64 1, i64 8, %complex_64bit* elementtype(%complex_64bit) getelementptr inbounds ([100 x %complex_64bit], [100 x %complex_64bit]* @pR, i32 0, i32 0), i64 %int_sext17)
  %pR_fetch = load %complex_64bit, %complex_64bit* %pR
  %int_cast = sitofp i32 %p0 to float
  %pR_fetch_c0 = extractvalue %complex_64bit %pR_fetch, 0
  %add = fadd float %pR_fetch_c0, %int_cast
  %pR_fetch_c1 = extractvalue %complex_64bit %pR_fetch, 1
  %add6 = fadd float %pR_fetch_c1, 0.000000e+00
  %insertval8 = insertvalue %complex_64bit zeroinitializer, float %add, 0
  %insertval10 = insertvalue %complex_64bit %insertval8, float %add6, 1
  store %complex_64bit %insertval10, %complex_64bit* %pR
  %add21 = add nsw i32 %p0, 1
  %rel = icmp sle i32 %add21, 5
  br i1 %rel, label %bb5, label %bb1

bb1:                                              ; preds = %bb5
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare %complex_64bit* @llvm.intel.subscript.p0s_complex_64bits.i64.i64.p0s_complex_64bits.i64(i8, i64, i64, %complex_64bit*, i64)

