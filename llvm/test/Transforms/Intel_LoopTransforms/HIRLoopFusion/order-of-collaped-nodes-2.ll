; RUN: opt -disable-hir-create-fusion-regions=0 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Fuse Graph before optimization
;
; Directed:
;  0:{ 84 } 64--> 1:{ 85 }
;  0:{ 84 } 320--> 3:{ 87 }
;  1:{ 85 } 64--> 2:{ 86 }
;  1:{ 85 } 128--> 3:{ 87 }
;  2:{ 86 } 128--> 3:{ 87 }

; Check that the order of nodes in the fused loop is (0 1 2 3)

; subroutine foo(t1, t2, t3, ina, ina2, ina3, ina4, c, o, y)
;   real, dimension(64) :: t1, t2, t3, ina, ina2, ina3, ina4, c, t, o, y
;
;   (0) t1 = ina + 1 + ina2 + ina3 + ina4
;   (1) t2 = t1 + 1
;   (2) t3 = t2 + 1
;   (3) c = t1 + t2 + t3 + ina + ina2 + ina3 + ina4
; end

; BEGIN REGION { }
;       + DO i1 = 0, 63, 1   <DO_LOOP>
;       |   %add = (%"foo_$INA")[i1]  +  1.000000e+00;
;       |   %add2 = %add  +  (%"foo_$INA2")[i1];
;       |   %add6 = %add2  +  (%"foo_$INA3")[i1];
;       |   %add10 = %add6  +  (%"foo_$INA4")[i1];
;       |   (%"foo_$T1")[i1] = %add10;
;       + END LOOP
;
;
;       + DO i1 = 0, 63, 1   <DO_LOOP>
;       |   %add23 = (%"foo_$T1")[i1]  +  1.000000e+00;
;       |   (%"foo_$T2")[i1] = %add23;
;       + END LOOP
;
;
;       + DO i1 = 0, 63, 1   <DO_LOOP>
;       |   %add37 = (%"foo_$T2")[i1]  +  1.000000e+00;
;       |   (%"foo_$T3")[i1] = %add37;
;       + END LOOP
;
;
;       + DO i1 = 0, 63, 1   <DO_LOOP>
;       |   %add54 = (%"foo_$T1")[i1]  +  (%"foo_$T2")[i1];
;       |   %add61 = %add54  +  (%"foo_$T3")[i1];
;       |   %add68 = %add61  +  (%"foo_$INA")[i1];
;       |   %add75 = %add68  +  (%"foo_$INA2")[i1];
;       |   %add82 = %add75  +  (%"foo_$INA3")[i1];
;       |   %add89 = %add82  +  (%"foo_$INA4")[i1];
;       |   (%"foo_$C")[i1] = %add89;
;       + END LOOP
; END REGION

; CHECK: modified
; CHECK:      + DO i1
; CHECK-NEXT: |   %add = (%"foo_$INA")[i1]  +  1.000000e+00;
; CHECK-NEXT: |   %add2 = %add  +  (%"foo_$INA2")[i1];
; CHECK-NEXT: |   %add6 = %add2  +  (%"foo_$INA3")[i1];
; CHECK-NEXT: |   %add10 = %add6  +  (%"foo_$INA4")[i1];
; CHECK-NEXT: |   (%"foo_$T1")[i1] = %add10;
; CHECK-NEXT: |   %add23 = (%"foo_$T1")[i1]  +  1.000000e+00;
; CHECK-NEXT: |   (%"foo_$T2")[i1] = %add23;
; CHECK-NEXT: |   %add37 = (%"foo_$T2")[i1]  +  1.000000e+00;
; CHECK-NEXT: |   (%"foo_$T3")[i1] = %add37;
; CHECK-NEXT: |   %add54 = (%"foo_$T1")[i1]  +  (%"foo_$T2")[i1];
; CHECK-NEXT: |   %add61 = %add54  +  (%"foo_$T3")[i1];
; CHECK-NEXT: |   %add68 = %add61  +  (%"foo_$INA")[i1];
; CHECK-NEXT: |   %add75 = %add68  +  (%"foo_$INA2")[i1];
; CHECK-NEXT: |   %add82 = %add75  +  (%"foo_$INA3")[i1];
; CHECK-NEXT: |   %add89 = %add82  +  (%"foo_$INA4")[i1];
; CHECK-NEXT: |   (%"foo_$C")[i1] = %add89;
; CHECK-NEXT: + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @foo_(ptr noalias nocapture dereferenceable(4) %"foo_$T1", ptr noalias nocapture dereferenceable(4) %"foo_$T2", ptr noalias nocapture dereferenceable(4) %"foo_$T3", ptr noalias nocapture readonly dereferenceable(4) %"foo_$INA", ptr noalias nocapture readonly dereferenceable(4) %"foo_$INA2", ptr noalias nocapture readonly dereferenceable(4) %"foo_$INA3", ptr noalias nocapture readonly dereferenceable(4) %"foo_$INA4", ptr noalias nocapture dereferenceable(4) %"foo_$C", ptr noalias nocapture readnone dereferenceable(4) %"foo_$O", ptr noalias nocapture readnone dereferenceable(4) %"foo_$Y") local_unnamed_addr #0 {
alloca_0:
  br label %bb20

bb20:                                             ; preds = %alloca_0, %bb20
  %"var$2.0124" = phi i64 [ 1, %alloca_0 ], [ %add18, %bb20 ]
  %"foo_$INA_entry[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$INA", i64 %"var$2.0124")
  %"foo_$INA_entry[]_fetch" = load float, ptr %"foo_$INA_entry[]", align 1
  %add = fadd float %"foo_$INA_entry[]_fetch", 1.000000e+00
  %"foo_$INA2_entry[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$INA2", i64 %"var$2.0124")
  %"foo_$INA2_entry[]_fetch" = load float, ptr %"foo_$INA2_entry[]", align 1
  %add2 = fadd float %add, %"foo_$INA2_entry[]_fetch"
  %"foo_$INA3_entry[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$INA3", i64 %"var$2.0124")
  %"foo_$INA3_entry[]_fetch" = load float, ptr %"foo_$INA3_entry[]", align 1
  %add6 = fadd float %add2, %"foo_$INA3_entry[]_fetch"
  %"foo_$INA4_entry[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$INA4", i64 %"var$2.0124")
  %"foo_$INA4_entry[]_fetch" = load float, ptr %"foo_$INA4_entry[]", align 1
  %add10 = fadd float %add6, %"foo_$INA4_entry[]_fetch"
  %"foo_$T1_entry[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$T1", i64 %"var$2.0124")
  store float %add10, ptr %"foo_$T1_entry[]", align 1
  %add18 = add nuw nsw i64 %"var$2.0124", 1
  %exitcond127 = icmp eq i64 %add18, 65
  br i1 %exitcond127, label %bb34.preheader, label %bb20

bb34.preheader:                                   ; preds = %bb20
  br label %bb34

bb34:                                             ; preds = %bb34.preheader, %bb34
  %"var$3.0123" = phi i64 [ %add32, %bb34 ], [ 1, %bb34.preheader ]
  %"foo_$T1_entry[]21" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$T1", i64 %"var$3.0123")
  %"foo_$T1_entry[]21_fetch" = load float, ptr %"foo_$T1_entry[]21", align 1
  %add23 = fadd float %"foo_$T1_entry[]21_fetch", 1.000000e+00
  %"foo_$T2_entry[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$T2", i64 %"var$3.0123")
  store float %add23, ptr %"foo_$T2_entry[]", align 1
  %add32 = add nuw nsw i64 %"var$3.0123", 1
  %exitcond126 = icmp eq i64 %add32, 65
  br i1 %exitcond126, label %bb48.preheader, label %bb34

bb48.preheader:                                   ; preds = %bb34
  br label %bb48

bb48:                                             ; preds = %bb48.preheader, %bb48
  %"var$4.0122" = phi i64 [ %add46, %bb48 ], [ 1, %bb48.preheader ]
  %"foo_$T2_entry[]35" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$T2", i64 %"var$4.0122")
  %"foo_$T2_entry[]35_fetch" = load float, ptr %"foo_$T2_entry[]35", align 1
  %add37 = fadd float %"foo_$T2_entry[]35_fetch", 1.000000e+00
  %"foo_$T3_entry[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$T3", i64 %"var$4.0122")
  store float %add37, ptr %"foo_$T3_entry[]", align 1
  %add46 = add nuw nsw i64 %"var$4.0122", 1
  %exitcond125 = icmp eq i64 %add46, 65
  br i1 %exitcond125, label %bb74.preheader, label %bb48

bb74.preheader:                                   ; preds = %bb48
  br label %bb74

bb74:                                             ; preds = %bb74.preheader, %bb74
  %"var$5.0121" = phi i64 [ %add99, %bb74 ], [ 1, %bb74.preheader ]
  %"foo_$T1_entry[]49" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$T1", i64 %"var$5.0121")
  %"foo_$T1_entry[]49_fetch" = load float, ptr %"foo_$T1_entry[]49", align 1
  %"foo_$T2_entry[]52" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$T2", i64 %"var$5.0121")
  %"foo_$T2_entry[]52_fetch" = load float, ptr %"foo_$T2_entry[]52", align 1
  %add54 = fadd float %"foo_$T1_entry[]49_fetch", %"foo_$T2_entry[]52_fetch"
  %"foo_$T3_entry[]59" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$T3", i64 %"var$5.0121")
  %"foo_$T3_entry[]59_fetch" = load float, ptr %"foo_$T3_entry[]59", align 1
  %add61 = fadd float %add54, %"foo_$T3_entry[]59_fetch"
  %"foo_$INA_entry[]66" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$INA", i64 %"var$5.0121")
  %"foo_$INA_entry[]66_fetch" = load float, ptr %"foo_$INA_entry[]66", align 1
  %add68 = fadd float %add61, %"foo_$INA_entry[]66_fetch"
  %"foo_$INA2_entry[]73" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$INA2", i64 %"var$5.0121")
  %"foo_$INA2_entry[]73_fetch" = load float, ptr %"foo_$INA2_entry[]73", align 1
  %add75 = fadd float %add68, %"foo_$INA2_entry[]73_fetch"
  %"foo_$INA3_entry[]80" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$INA3", i64 %"var$5.0121")
  %"foo_$INA3_entry[]80_fetch" = load float, ptr %"foo_$INA3_entry[]80", align 1
  %add82 = fadd float %add75, %"foo_$INA3_entry[]80_fetch"
  %"foo_$INA4_entry[]87" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$INA4", i64 %"var$5.0121")
  %"foo_$INA4_entry[]87_fetch" = load float, ptr %"foo_$INA4_entry[]87", align 1
  %add89 = fadd float %add82, %"foo_$INA4_entry[]87_fetch"
  %"foo_$C_entry[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(float) nonnull %"foo_$C", i64 %"var$5.0121")
  store float %add89, ptr %"foo_$C_entry[]", align 1
  %add99 = add nuw nsw i64 %"var$5.0121", 1
  %exitcond = icmp eq i64 %add99, 65
  br i1 %exitcond, label %bb75, label %bb74

bb75:                                             ; preds = %bb74
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="corei7-avx" "target-features"="+avx,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nounwind readnone speculatable }

