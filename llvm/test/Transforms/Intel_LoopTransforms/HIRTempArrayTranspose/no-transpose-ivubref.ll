; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-temp-array-transpose,print<hir>" -disable-output -hir-temp-array-transpose-allow-unknown-sizes 2>&1 | FileCheck %s

; Check that transposing does not not occur for the Loop IV UBRef corresponding to:
; DO i2 = 0, i1, 1
; We cannot create a transpose loop using a UBRef with an IV.
; Def@level is set to 0 for the IV UBRef in this case.

; HIR Before

;       BEGIN REGION { }
;             + DO i1 = 0, %"arbrcs_$NDIM_fetch.2341" + -1, 1   <DO_LOOP>
;             |   + DO i2 = 0, i1, 1   <DO_LOOP>
;             |   |   %"arbrcs_$X.5" = 0.000000e+00;
;             |   |
;             |   |      %"arbrcs_$X.4" = 0.000000e+00;
;             |   |   + DO i3 = 0, %"arbrcs_$NLINK_fetch.2345" + -1, 1   <DO_LOOP>
;             |   |   |   %mul.243 = (%"arbrcs_$B")[i2][i3]  *  (%"arbrcs_$A")[i1][i3];
;             |   |   |   %"arbrcs_$X.4" = %mul.243  +  %"arbrcs_$X.4";
;             |   |   + END LOOP
;             |   |      %"arbrcs_$X.5" = %"arbrcs_$X.4";
;             |   |
;             |   |   %mul.244 = (%"arbrcs_$C")[i2][i1]  *  %"arbrcs_$FC_fetch.2361";
;             |   |   %mul.245 = %"arbrcs_$FA_fetch.2367"  *  %"arbrcs_$X.5";
;             |   |   %add.328 = %mul.245  +  %mul.244;
;             |   |   (%"arbrcs_$C")[i2][i1] = %add.328;
;             |   |   (%"arbrcs_$C")[i1][i2] = %add.328;
;             |   + END LOOP
;             + END LOOP
;       END REGION


; CHECK:  BEGIN REGION
; CHECK-NOT: modified

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

; Function Attrs: nofree noinline nounwind uwtable
define dso_local i1 @arbrcs_.bb_new1967_then(ptr %"arbrcs_$NDIM", ptr %"arbrcs_$NLINK", ptr %"arbrcs_$FC", ptr %"arbrcs_$FA", i64 %mul.236, ptr %"arbrcs_$C", i64 %mul.231, ptr %"arbrcs_$A", i64 %mul.233, ptr %"arbrcs_$B") #1 {
newFuncRoot:
  br label %bb_new1967_then

bb_new1967_then:                                  ; preds = %newFuncRoot
  %"arbrcs_$NDIM_fetch.2341" = load i64, ptr %"arbrcs_$NDIM", align 1
  %rel.424 = icmp slt i64 %"arbrcs_$NDIM_fetch.2341", 1
  br i1 %rel.424, label %bb721_endif.exitStub, label %bb692.preheader

bb692.preheader:                                  ; preds = %bb_new1967_then
  %"arbrcs_$NLINK_fetch.2345" = load i64, ptr %"arbrcs_$NLINK", align 1
  %rel.426 = icmp slt i64 %"arbrcs_$NLINK_fetch.2345", 1
  %"arbrcs_$FC_fetch.2361" = load double, ptr %"arbrcs_$FC", align 1
  %"arbrcs_$FA_fetch.2367" = load double, ptr %"arbrcs_$FA", align 1
  %0 = add nsw i64 %"arbrcs_$NLINK_fetch.2345", 1
  %1 = add nuw i64 %"arbrcs_$NDIM_fetch.2341", 2
  br label %bb692

bb692:                                            ; preds = %bb692.preheader, %bb697
  %indvars.iv248 = phi i64 [ 2, %bb692.preheader ], [ %indvars.iv.next249, %bb697 ]
  %"arbrcs_$I.2" = phi i64 [ 1, %bb692.preheader ], [ %add.330, %bb697 ]
  %"arbrcs_$C[]67" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.236, ptr nonnull elementtype(double) %"arbrcs_$C", i64 %"arbrcs_$I.2")
  %"arbrcs_$A[]57" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.231, ptr nonnull elementtype(double) %"arbrcs_$A", i64 %"arbrcs_$I.2")
  br label %bb696

bb696:                                            ; preds = %bb701, %bb692
  %"arbrcs_$J.2" = phi i64 [ 1, %bb692 ], [ %add.329, %bb701 ]
  br i1 %rel.426, label %bb701, label %bb700.preheader

bb700.preheader:                                  ; preds = %bb696
  %"arbrcs_$B[]59" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.233, ptr nonnull elementtype(double) %"arbrcs_$B", i64 %"arbrcs_$J.2")
  br label %bb700

bb700:                                            ; preds = %bb700, %bb700.preheader
  %"arbrcs_$K.2" = phi i64 [ %add.327, %bb700 ], [ 1, %bb700.preheader ]
  %"arbrcs_$X.4" = phi double [ %add.326, %bb700 ], [ 0.000000e+00, %bb700.preheader ]
  %"arbrcs_$A[][]58" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"arbrcs_$A[]57", i64 %"arbrcs_$K.2")
  %"arbrcs_$A[][]_fetch.2352" = load double, ptr %"arbrcs_$A[][]58", align 1
  %"arbrcs_$B[][]60" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"arbrcs_$B[]59", i64 %"arbrcs_$K.2")
  %"arbrcs_$B[][]_fetch.2357" = load double, ptr %"arbrcs_$B[][]60", align 1
  %mul.243 = fmul reassoc ninf nsz arcp contract afn double %"arbrcs_$B[][]_fetch.2357", %"arbrcs_$A[][]_fetch.2352"
  %add.326 = fadd reassoc ninf nsz arcp contract afn double %mul.243, %"arbrcs_$X.4"
  %add.327 = add nuw nsw i64 %"arbrcs_$K.2", 1
  %exitcond247 = icmp eq i64 %add.327, %0
  br i1 %exitcond247, label %bb701.loopexit, label %bb700

bb701.loopexit:                                   ; preds = %bb700
  %add.326.lcssa = phi double [ %add.326, %bb700 ]
  br label %bb701

bb701:                                            ; preds = %bb701.loopexit, %bb696
  %"arbrcs_$X.5" = phi double [ 0.000000e+00, %bb696 ], [ %add.326.lcssa, %bb701.loopexit ]
  %"arbrcs_$C[]61" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.236, ptr nonnull elementtype(double) %"arbrcs_$C", i64 %"arbrcs_$J.2")
  %"arbrcs_$C[][]62" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"arbrcs_$C[]61", i64 %"arbrcs_$I.2")
  %"arbrcs_$C[][]_fetch.2366" = load double, ptr %"arbrcs_$C[][]62", align 1
  %mul.244 = fmul reassoc ninf nsz arcp contract afn double %"arbrcs_$C[][]_fetch.2366", %"arbrcs_$FC_fetch.2361"
  %mul.245 = fmul reassoc ninf nsz arcp contract afn double %"arbrcs_$FA_fetch.2367", %"arbrcs_$X.5"
  %add.328 = fadd reassoc ninf nsz arcp contract afn double %mul.245, %mul.244
  store double %add.328, ptr %"arbrcs_$C[][]62", align 1
  %"arbrcs_$C[][]68" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"arbrcs_$C[]67", i64 %"arbrcs_$J.2")
  store double %add.328, ptr %"arbrcs_$C[][]68", align 1
  %add.329 = add nuw nsw i64 %"arbrcs_$J.2", 1
  %exitcond250 = icmp eq i64 %add.329, %indvars.iv248
  br i1 %exitcond250, label %bb697, label %bb696

bb697:                                            ; preds = %bb701
  %add.330 = add nuw nsw i64 %"arbrcs_$I.2", 1
  %indvars.iv.next249 = add nuw i64 %indvars.iv248, 1
  %exitcond251 = icmp eq i64 %indvars.iv.next249, %1
  br i1 %exitcond251, label %bb721_endif.loopexit260.exitStub, label %bb692

bb721_endif.exitStub:                             ; preds = %bb_new1967_then
  ret i1 true

bb721_endif.loopexit260.exitStub:                 ; preds = %bb697
  ret i1 false
}

!omp_offload.info = !{}