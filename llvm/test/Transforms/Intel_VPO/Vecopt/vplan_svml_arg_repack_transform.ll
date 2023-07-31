; Test to verify correctness of the VPlan transform that performs
; argument repacking for complex type function calls that will be
; vectorized using SVML.

; RUN: opt -passes=vplan-vec -vplan-force-vf=2 -vector-library=SVML -vplan-print-after-transformed-library-calls -S < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,IR
; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>' -vplan-force-vf=2 -vector-library=SVML -vplan-print-after-transformed-library-calls -disable-output < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,HIR

; Checks for VPlan IR after transform
; CHECK-LABEL:  VPlan after transforming library calls:
; CHECK:           [[LOOP:BB[0-9]+]]:
; CHECK:            [DA: Div] <2 x double> [[VP_CWAR_ARG_0:%.*]] = insertelement <2 x double> poison double [[VP_A_REAL:%.*]] i64 0
; CHECK-NEXT:       [DA: Div] <2 x double> [[VP_CWAR_ARG_1:%.*]] = insertelement <2 x double> [[VP_CWAR_ARG_0]] double [[VP_A_IMAG:%.*]] i64 1
; CHECK-NEXT:       [DA: Div] <2 x double> [[VP_TRANSFORMED:%.*]] = transform-lib-call <2 x double> [[VP_CWAR_ARG_1]] __svml_clog2 [x 1]
; CHECK-NOT:        extractvalue
; CHECK-NEXT:       [DA: Div] double [[VP1:%.*]] = extractelement <2 x double> [[VP_TRANSFORMED]] i64 0
; CHECK-NEXT:       [DA: Div] double [[VP2:%.*]] = extractelement <2 x double> [[VP_TRANSFORMED]] i64 1

; CHECK:           [[IF_THEN:BB[0-9]+]]:
; CHECK-NEXT:       [DA: Div] i1 [[VP3:%.*]] = block-predicate i1 [[VP_COND:%.*]]
; CHECK-NEXT:       [DA: Div] <2 x double> [[VP_CWAR_ARG_0_1:%.*]] = insertelement <2 x double> poison double [[VP_A_REAL]] i64 0
; CHECK-NEXT:       [DA: Div] <2 x double> [[VP_CWAR_ARG_1_1:%.*]] = insertelement <2 x double> [[VP_CWAR_ARG_0_1]] double [[VP_A_IMAG]] i64 1
; CHECK-NEXT:       [DA: Div] <2 x double> [[VP_TRANSFORMED_1:%.*]] = transform-lib-call <2 x double> [[VP_CWAR_ARG_1_1]] __svml_cexp2_mask [x 1] [@CurrMask]
; CHECK-NOT:        extractvalue
; CHECK-NEXT:       [DA: Div] double [[VP4:%.*]] = extractelement <2 x double> [[VP_TRANSFORMED_1]] i64 0
; CHECK-NEXT:       [DA: Div] double [[VP5:%.*]] = extractelement <2 x double> [[VP_TRANSFORMED_1]] i64 1
;
; Checks for LLVM-IR codegen.
; IR:           define void @foo(ptr noalias nocapture [[A0:%.*]]) {
; IR:             [[TMP0:%.*]] = call fast svml_cc <4 x double> @__svml_clog2(<4 x double> [[WIDE_INSERT60:%.*]])

; IR:             [[MASK_CMP:%.*]] = icmp eq <2 x i64> [[IV_PHI:%.*]], <i64 42, i64 42>
; IR:             [[TMP5:%.*]] = sext <2 x i1> [[MASK_CMP]] to <2 x i64>
; IR:             [[TMP6:%.*]] = call fast svml_cc <4 x double> @__svml_cexp2_mask(<4 x double> [[WIDE_INSERT120:%.*]], <2 x i64> [[TMP5]])
;
; Checks for HIR codegen.
; HIR:   BEGIN REGION { modified }
; HIR:         + DO i1 = 0, 1023, 2   <DO_LOOP> <simd-vectorized> <novectorize>
; HIR:         |   %__svml_clog2 = @__svml_clog2(%wide.insert3);
; HIR:         |   %.vec = i1 + <i64 0, i64 1> == 42;
; HIR:         |   %sext = sext.<2 x i1>.<2 x i64>(%.vec);
; HIR:         |   %__svml_cexp2_mask = @__svml_cexp2_mask(%wide.insert7,  %sext);
; HIR:         + END LOOP

; HIR:         + DO i1 = 1024, 1024, 1   <DO_LOOP> <vector-remainder> <novectorize>
; HIR:         |   %log = @clog(%A.real,  %A.imag);
; HIR:         |   %exp = @cexp(%A.real,  %A.imag);
; HIR:         + END LOOP
; HIR:   END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%complex_128bit = type { double, double }

define void @foo(ptr nocapture noalias %A) {
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %if.merge ]
  %A.idx = getelementptr inbounds %complex_128bit, ptr %A, i64 %iv
  %A.idx.imag = getelementptr %complex_128bit, ptr %A.idx, i64 0, i32 1
  %A.real = load double, ptr %A.idx, align 1
  %A.imag = load double, ptr %A.idx.imag, align 1
  %log = tail call fast %complex_128bit @clog(double %A.real, double %A.imag)
  %log.real = extractvalue %complex_128bit %log, 0
  %log.imag = extractvalue %complex_128bit %log, 1
  store double %log.real, ptr %A.idx, align 1
  store double %log.imag, ptr %A.idx.imag, align 1
  %cond = icmp eq i64 %iv, 42
  br i1 %cond, label %if.then, label %if.merge

if.then:
  %exp = tail call fast %complex_128bit @cexp(double %A.real, double %A.imag)
  %exp.real = extractvalue %complex_128bit %exp, 0
  %exp.imag = extractvalue %complex_128bit %exp, 1
  store double %exp.real, ptr %A.idx, align 1
  store double %exp.imag, ptr %A.idx.imag, align 1
  br label %if.merge

if.merge:
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv, 1024
  br i1 %exitcond, label %exit, label %loop

exit:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token %0)

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare %complex_128bit @clog(double, double) local_unnamed_addr #0

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare %complex_128bit @cexp(double, double) local_unnamed_addr #0

attributes #0 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
