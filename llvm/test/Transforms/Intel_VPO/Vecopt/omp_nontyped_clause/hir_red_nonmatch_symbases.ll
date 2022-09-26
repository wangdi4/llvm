; Test that we bail out on reduction that has different symbases for livein
; and liveout temps. We create the initial vplan but don't process it
; further.
; The incoming IR looks like below, pay attention that incoming temp of
; reduction is %3 and outgoing is %add7.reload. That is a deficiency in
; loopopt. The test should be changed when it's fixed.
;
; BEGIN REGION { }
;   %1 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.REDUCTION.ADD(&((%red.red)[0])),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null),  QUAL.OMP.PRIVATE(&((%add7.loc)[0])) ]
;   %red.red.promoted = (%red.red)[0];
;   %3 = %red.red.promoted;
;
;   + DO i1 = 0, zext.i32.i64(%0) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647> <simd>
;   |   @foo(i1,  %3,  &((%add7.loc)[0]));
;   |   %add7.reload = (%add7.loc)[0];
;   |   %3 = %add7.reload;
;   + END LOOP
;
;   (%red.red)[0] = %add7.reload;
;   @llvm.directive.region.exit(%1); [ DIR.OMP.END.SIMD() ]
; END REGION
;
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-temp-cleanup -hir-last-value-computation -hir-vplan-vec -disable-vplan-codegen -vplan-entities-dump -vplan-print-after-vpentity-instrs -vplan-print-after-plain-cfg -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-vec-dir-insert,hir-vplan-vec" -disable-vplan-codegen -vplan-entities-dump -vplan-print-after-vpentity-instrs -vplan-print-after-plain-cfg -disable-output < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

; Check entities dump and VPlan IR
; CHECK: Reduction list
; CHECK-NEXT:   (+) Start: i8 [[TMP3:%.*]] Exit: i8 [[VP_LOAD:%.*]]
; CHECK-NEXT:    Linked values: i8 [[VP4:%.*]], i8 [[VP_LOAD]], i8 [[VP5:%.*]],
; CHECK-NOT: reduction-init

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_ZN11OrderedTest10run_vectorEv(i32* nocapture readonly %c_size, i8** nocapture readonly %m_in, i8** nocapture readonly %m_out_vec) local_unnamed_addr #5 align 2 {
entry:
  %add7.loc = alloca i8, align 1
  %red.red = alloca i8, align 1
  %0 = load i32, i32* %c_size, align 8
  %cmp = icmp sgt i32 %0, 0
  br i1 %cmp, label %DIR.OMP.SIMD.1, label %omp.precond.end

DIR.OMP.SIMD.1:                                   ; preds = %entry
  store i8 0, i8* %red.red, align 1
  br label %DIR.OMP.SIMD.139

DIR.OMP.SIMD.139:                                 ; preds = %DIR.OMP.SIMD.1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD"(i8* %red.red), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.PRIVATE"(i8* %add7.loc) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.139
  %2 = bitcast i32* %c_size to i8*
  %red.red.promoted = load i8, i8* %red.red, align 1
  %wide.trip.count38 = zext i32 %0 to i64
  br label %DIR.OMP.END.ORDERED.337

DIR.OMP.END.ORDERED.337:                          ; preds = %DIR.OMP.SIMD.2, %DIR.OMP.END.ORDERED.6
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %DIR.OMP.END.ORDERED.6 ]
  %3 = phi i8 [ %red.red.promoted, %DIR.OMP.SIMD.2 ], [ %add7.reload, %DIR.OMP.END.ORDERED.6 ]
  br label %codeRepl

codeRepl:                                         ; preds = %DIR.OMP.END.ORDERED.337
  call void @foo(i64 %indvars.iv, i8 %3, i8* %add7.loc) #0
  %add7.reload = load i8, i8* %add7.loc, align 1
  br label %DIR.OMP.END.ORDERED.6

DIR.OMP.END.ORDERED.6:                            ; preds = %codeRepl
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count38
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.4, label %DIR.OMP.END.ORDERED.337

DIR.OMP.END.SIMD.4:                               ; preds = %DIR.OMP.END.ORDERED.6
  %add7.lcssa = phi i8 [ %add7.reload, %DIR.OMP.END.ORDERED.6 ]
  store i8 %add7.lcssa, i8* %red.red, align 1
  br label %DIR.OMP.END.SIMD.7

DIR.OMP.END.SIMD.7:                               ; preds = %DIR.OMP.END.SIMD.4
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.7, %entry
  %red.1 = phi i8 [ 0, %entry ], [ %add7.lcssa, %DIR.OMP.END.SIMD.7 ]
  %4 = load i8*, i8** %m_out_vec, align 8
  store i8 %red.1, i8* %4, align 1
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #6

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #6

declare dso_local void @foo(i64, i8, i8*)

attributes #0 = { nounwind }
