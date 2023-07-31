; RUN: opt -disable-hir-create-fusion-regions=0 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Fuse Graph before optimization
;
; Undirected:
;  0:{ 86 } 64--> 1:{ 87 }
;  1:{ 87 } 64--> 0:{ 86 }
;  1:{ 87 } 64--> 2:{ 88 }
;  2:{ 88 } 64--> 1:{ 87 }
;
; Directed:
;  0:{ 86 } 128--> 2:{ 88 }
;  0:{ 86 } 64--> 3:{ 89 }
;  1:{ 87 } 256--> 3:{ 89 }
;  2:{ 88 } 192--> 3:{ 89 }

; Check that the order of nodes in the fused loop is (0 2 1 3)

;  subroutine foo(t1, t2, t3, ina, ina2, ina3, ina4, c)
;    real, dimension(64) :: t1, t2, t3, ina, ina2, ina3, ina4, c
;
;    (0) t1 = ina + 1
;    (1) t2 = ina + 2 + ina2 + ina3 + ina4
;    (2) t3 = ina + c + t1
;    (3) c = t1 + t2 + t3 + ina2 + ina3 + ina4
;
;  end

; BEGIN REGION { }
;       + DO i1 = 0, 63, 1   <DO_LOOP>
;       |   %add = (%"foo_$INA")[i1]  +  1.000000e+00;
;       |   (%"foo_$T1")[i1] = %add;
;       + END LOOP
;
;
;       + DO i1 = 0, 63, 1   <DO_LOOP>
;       |   %add12 = (%"foo_$INA")[i1]  +  2.000000e+00;
;       |   %add14 = %add12  +  (%"foo_$INA2")[i1];
;       |   %add18 = %add14  +  (%"foo_$INA3")[i1];
;       |   %add22 = %add18  +  (%"foo_$INA4")[i1];
;       |   (%"foo_$T2")[i1] = %add22;
;       + END LOOP
;
;
;       + DO i1 = 0, 63, 1   <DO_LOOP>
;       |   %add37 = (%"foo_$INA")[i1]  +  (%"foo_$C")[i1];
;       |   %add44 = %add37  +  (%"foo_$T1")[i1];
;       |   (%"foo_$T3")[i1] = %add44;
;       + END LOOP
;
;
;       + DO i1 = 0, 63, 1   <DO_LOOP>
;       |   %add65 = (%"foo_$T1")[i1]  +  (%"foo_$T2")[i1];
;       |   %add72 = %add65  +  (%"foo_$T3")[i1];
;       |   %add79 = %add72  +  (%"foo_$INA2")[i1];
;       |   %add86 = %add79  +  (%"foo_$INA3")[i1];
;       |   %add93 = %add86  +  (%"foo_$INA4")[i1];
;       |   (%"foo_$C")[i1] = %add93;
;       + END LOOP
; END REGION

; CHECK: modified
; CHECK:      + DO i1
; CHECK-NEXT: |   %add = (%"foo_$INA")[i1]  +  1.000000e+00;
; CHECK-NEXT: |   (%"foo_$T1")[i1] = %add;
; CHECK-NEXT: |   %add37 = (%"foo_$INA")[i1]  +  (%"foo_$C")[i1];
; CHECK-NEXT: |   %add44 = %add37  +  (%"foo_$T1")[i1];
; CHECK-NEXT: |   (%"foo_$T3")[i1] = %add44;
; CHECK-NEXT: |   %add12 = (%"foo_$INA")[i1]  +  2.000000e+00;
; CHECK-NEXT: |   %add14 = %add12  +  (%"foo_$INA2")[i1];
; CHECK-NEXT: |   %add18 = %add14  +  (%"foo_$INA3")[i1];
; CHECK-NEXT: |   %add22 = %add18  +  (%"foo_$INA4")[i1];
; CHECK-NEXT: |   (%"foo_$T2")[i1] = %add22;
; CHECK-NEXT: |   %add65 = (%"foo_$T1")[i1]  +  (%"foo_$T2")[i1];
; CHECK-NEXT: |   %add72 = %add65  +  (%"foo_$T3")[i1];
; CHECK-NEXT: |   %add79 = %add72  +  (%"foo_$INA2")[i1];
; CHECK-NEXT: |   %add86 = %add79  +  (%"foo_$INA3")[i1];
; CHECK-NEXT: |   %add93 = %add86  +  (%"foo_$INA4")[i1];
; CHECK-NEXT: |   (%"foo_$C")[i1] = %add93;
; CHECK-NEXT: + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @foo_(ptr noalias nocapture dereferenceable(4) %"foo_$T1", ptr noalias nocapture dereferenceable(4) %"foo_$T2", ptr noalias nocapture dereferenceable(4) %"foo_$T3", ptr noalias nocapture readonly dereferenceable(4) %"foo_$INA", ptr noalias nocapture readonly dereferenceable(4) %"foo_$INA2", ptr noalias nocapture readonly dereferenceable(4) %"foo_$INA3", ptr noalias nocapture readonly dereferenceable(4) %"foo_$INA4", ptr noalias nocapture dereferenceable(4) %"foo_$C") local_unnamed_addr #0 {
alloca_0:
  br label %bb14

bb14:                                             ; preds = %alloca_0, %bb14
  %"var$2.0129" = phi i64 [ 1, %alloca_0 ], [ %add7, %bb14 ]
  %"foo_$INA_entry[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$INA", i64 %"var$2.0129")
  %"foo_$INA_entry[]_fetch" = load float, ptr %"foo_$INA_entry[]", align 1
  %add = fadd float %"foo_$INA_entry[]_fetch", 1.000000e+00
  %"foo_$T1_entry[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$T1", i64 %"var$2.0129")
  store float %add, ptr %"foo_$T1_entry[]", align 1
  %add7 = add nuw nsw i64 %"var$2.0129", 1
  %exitcond132 = icmp eq i64 %add7, 65
  br i1 %exitcond132, label %bb34.preheader, label %bb14

bb34.preheader:                                   ; preds = %bb14
  br label %bb34

bb34:                                             ; preds = %bb34.preheader, %bb34
  %"var$3.0128" = phi i64 [ %add32, %bb34 ], [ 1, %bb34.preheader ]
  %"foo_$INA_entry[]10" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$INA", i64 %"var$3.0128")
  %"foo_$INA_entry[]10_fetch" = load float, ptr %"foo_$INA_entry[]10", align 1
  %add12 = fadd float %"foo_$INA_entry[]10_fetch", 2.000000e+00
  %"foo_$INA2_entry[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$INA2", i64 %"var$3.0128")
  %"foo_$INA2_entry[]_fetch" = load float, ptr %"foo_$INA2_entry[]", align 1
  %add14 = fadd float %add12, %"foo_$INA2_entry[]_fetch"
  %"foo_$INA3_entry[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$INA3", i64 %"var$3.0128")
  %"foo_$INA3_entry[]_fetch" = load float, ptr %"foo_$INA3_entry[]", align 1
  %add18 = fadd float %add14, %"foo_$INA3_entry[]_fetch"
  %"foo_$INA4_entry[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$INA4", i64 %"var$3.0128")
  %"foo_$INA4_entry[]_fetch" = load float, ptr %"foo_$INA4_entry[]", align 1
  %add22 = fadd float %add18, %"foo_$INA4_entry[]_fetch"
  %"foo_$T2_entry[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$T2", i64 %"var$3.0128")
  store float %add22, ptr %"foo_$T2_entry[]", align 1
  %add32 = add nuw nsw i64 %"var$3.0128", 1
  %exitcond131 = icmp eq i64 %add32, 65
  br i1 %exitcond131, label %bb52.preheader, label %bb34

bb52.preheader:                                   ; preds = %bb34
  br label %bb52

bb52:                                             ; preds = %bb52.preheader, %bb52
  %"var$4.0127" = phi i64 [ %add54, %bb52 ], [ 1, %bb52.preheader ]
  %"foo_$INA_entry[]35" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$INA", i64 %"var$4.0127")
  %"foo_$INA_entry[]35_fetch" = load float, ptr %"foo_$INA_entry[]35", align 1
  %"foo_$C_entry[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$C", i64 %"var$4.0127")
  %"foo_$C_entry[]_fetch" = load float, ptr %"foo_$C_entry[]", align 1
  %add37 = fadd float %"foo_$INA_entry[]35_fetch", %"foo_$C_entry[]_fetch"
  %"foo_$T1_entry[]42" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$T1", i64 %"var$4.0127")
  %"foo_$T1_entry[]42_fetch" = load float, ptr %"foo_$T1_entry[]42", align 1
  %add44 = fadd float %add37, %"foo_$T1_entry[]42_fetch"
  %"foo_$T3_entry[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$T3", i64 %"var$4.0127")
  store float %add44, ptr %"foo_$T3_entry[]", align 1
  %add54 = add nuw nsw i64 %"var$4.0127", 1
  %exitcond130 = icmp eq i64 %add54, 65
  br i1 %exitcond130, label %bb76.preheader, label %bb52

bb76.preheader:                                   ; preds = %bb52
  br label %bb76

bb76:                                             ; preds = %bb76.preheader, %bb76
  %"var$5.0126" = phi i64 [ %add103, %bb76 ], [ 1, %bb76.preheader ]
  %"foo_$T1_entry[]60" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$T1", i64 %"var$5.0126")
  %"foo_$T1_entry[]60_fetch" = load float, ptr %"foo_$T1_entry[]60", align 1
  %"foo_$T2_entry[]63" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$T2", i64 %"var$5.0126")
  %"foo_$T2_entry[]63_fetch" = load float, ptr %"foo_$T2_entry[]63", align 1
  %add65 = fadd float %"foo_$T1_entry[]60_fetch", %"foo_$T2_entry[]63_fetch"
  %"foo_$T3_entry[]70" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$T3", i64 %"var$5.0126")
  %"foo_$T3_entry[]70_fetch" = load float, ptr %"foo_$T3_entry[]70", align 1
  %add72 = fadd float %add65, %"foo_$T3_entry[]70_fetch"
  %"foo_$INA2_entry[]77" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$INA2", i64 %"var$5.0126")
  %"foo_$INA2_entry[]77_fetch" = load float, ptr %"foo_$INA2_entry[]77", align 1
  %add79 = fadd float %add72, %"foo_$INA2_entry[]77_fetch"
  %"foo_$INA3_entry[]84" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$INA3", i64 %"var$5.0126")
  %"foo_$INA3_entry[]84_fetch" = load float, ptr %"foo_$INA3_entry[]84", align 1
  %add86 = fadd float %add79, %"foo_$INA3_entry[]84_fetch"
  %"foo_$INA4_entry[]91" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$INA4", i64 %"var$5.0126")
  %"foo_$INA4_entry[]91_fetch" = load float, ptr %"foo_$INA4_entry[]91", align 1
  %add93 = fadd float %add86, %"foo_$INA4_entry[]91_fetch"
  %"foo_$C_entry[]57" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$C", i64 %"var$5.0126")
  store float %add93, ptr %"foo_$C_entry[]57", align 1
  %add103 = add nuw nsw i64 %"var$5.0126", 1
  %exitcond = icmp eq i64 %add103, 65
  br i1 %exitcond, label %bb77, label %bb76

bb77:                                             ; preds = %bb76
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="corei7-avx" "target-features"="+avx,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}
