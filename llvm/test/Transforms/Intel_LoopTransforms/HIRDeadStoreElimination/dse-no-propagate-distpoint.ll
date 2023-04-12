; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-dead-store-elimination,print<hir>" -disable-output 2>&1 < %s | FileCheck %s

; Verify that we do not optimize away single use load with distribute point:
;   %"sub1_$II[]_fetch.54" = (@"sub1_$II")[i1]; <distribute_point>
; %"sub1_$II[]_fetch.54" would otherwise be safe to optimize away but without
; a valid directive, Loop Distribution will assert later.

; Print Before DSE

; CHECK: BEGIN REGION { }
;           + DO i1 = 0, sext.i32.i64((-1 + %"sub1_$N_fetch.18")), 1   <DO_LOOP>
;           |   %"sub1_$A[]_fetch.50" = (%"sub1_$A")[i1];
;           |   %add.13 = %"sub1_$A[]_fetch.50"  +  %add.12;
;           |   (%"sub1_$A")[i1] = %add.13;
;           |   %"sub1_$II[]_fetch.54" = (@"sub1_$II")[i1]; <distribute_point>
;           |   %"(float)sub1_$K_fetch.57$" = sitofp.i32.float(%"sub1_$II[]_fetch.54");
;           |   %add.14 = %add.13  +  %"(float)sub1_$K_fetch.57$";
;           |   (%"sub1_$A")[i1] = %add.14;
;           + END LOOP
;     END REGION

; Print After DSE

; CHECK: BEGIN REGION { modified }
; CHECK:    + DO i1 = 0, sext.i32.i64((-1 + %"sub1_$N_fetch.18")), 1   <DO_LOOP>
;           |   %"sub1_$A[]_fetch.50" = (%"sub1_$A")[i1];
;           |   %add.13 = %"sub1_$A[]_fetch.50"  +  %add.12;
; CHECK-NOT:|   (%"sub1_$A")[i1] = %add.13;
; CHECK:    |   %"sub1_$II[]_fetch.54" = (@"sub1_$II")[i1]; <distribute_point>
;           |   %"(float)sub1_$K_fetch.57$" = sitofp.i32.float(%"sub1_$II[]_fetch.54");
;           |   %add.14 = %add.13  +  %"(float)sub1_$K_fetch.57$";
;           |   (%"sub1_$A")[i1] = %add.14;
;           + END LOOP
;     END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"sub1_$II" = external global [100 x i32]

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

define void @sub1_(ptr noalias %"sub1_$A", float %add.12) #1 {
alloca_1:
  %"sub1_$N_fetch.18" = load i32, ptr null, align 1
  br i1 true, label %bb30, label %bb17.preheader

bb17.preheader:                                   ; preds = %alloca_1
  %0 = add nsw i32 %"sub1_$N_fetch.18", 1
  %wide.trip.count = zext i32 %0 to i64
  br label %bb17

bb17:                                             ; preds = %bb17, %bb17.preheader
  %exitcond = icmp eq i64 0, %wide.trip.count
  br i1 %exitcond, label %DIR.PRAGMA.DISTRIBUTE_POINT.1, label %bb17

DIR.PRAGMA.DISTRIBUTE_POINT.1:                    ; preds = %bb17
  %1 = call token @llvm.directive.region.entry()
  br label %DIR.PRAGMA.END.DISTRIBUTE_POINT.4

DIR.PRAGMA.END.DISTRIBUTE_POINT.4:                ; preds = %DIR.PRAGMA.DISTRIBUTE_POINT.5, %DIR.PRAGMA.DISTRIBUTE_POINT.1
  %indvars.iv104 = phi i64 [ %indvars.iv.next105, %DIR.PRAGMA.DISTRIBUTE_POINT.5 ], [ 1, %DIR.PRAGMA.DISTRIBUTE_POINT.1 ]
  %"sub1_$A[]18" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 0, ptr elementtype(float) %"sub1_$A", i64 %indvars.iv104)
  %"sub1_$A[]_fetch.50" = load float, ptr %"sub1_$A[]18", align 1
  %add.13 = fadd float %"sub1_$A[]_fetch.50", %add.12
  store float %add.13, ptr %"sub1_$A[]18", align 1
  br label %DIR.PRAGMA.DISTRIBUTE_POINT.5

DIR.PRAGMA.DISTRIBUTE_POINT.5:                    ; preds = %DIR.PRAGMA.END.DISTRIBUTE_POINT.4
  %2 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.DISTRIBUTE_POINT"() ]
  %"sub1_$II[]22" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 0, ptr elementtype(i32) @"sub1_$II", i64 %indvars.iv104)
  %"sub1_$II[]_fetch.54" = load i32, ptr %"sub1_$II[]22", align 4
  %"(float)sub1_$K_fetch.57$" = sitofp i32 %"sub1_$II[]_fetch.54" to float
  %add.14 = fadd float %add.13, %"(float)sub1_$K_fetch.57$"
  store float %add.14, ptr %"sub1_$A[]18", align 1
  %indvars.iv.next105 = add i64 %indvars.iv104, 1
  %3 = trunc i64 %indvars.iv.next105 to i32
  %rel.10.not = icmp slt i32 %"sub1_$N_fetch.18", %3
  br i1 %rel.10.not, label %DIR.PRAGMA.END.DISTRIBUTE_POINT.7110.preheader, label %DIR.PRAGMA.END.DISTRIBUTE_POINT.4

DIR.PRAGMA.END.DISTRIBUTE_POINT.7110.preheader:   ; preds = %DIR.PRAGMA.DISTRIBUTE_POINT.5
  ret void

bb30:                                             ; preds = %alloca_1
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; uselistorder directives
uselistorder ptr @llvm.directive.region.entry, { 1, 0 }

attributes #0 = { nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
attributes #1 = { "intel-lang"="fortran" }
attributes #2 = { nounwind }
