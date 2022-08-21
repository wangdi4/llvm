; Test to verify that invalid PHIs are not emitted to track
; index for conditional last private finalization by
; VPLoopEntities framework.

; RUN: opt -disable-output %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vplan-vec -vplan-enable-inmemory-entities -vplan-print-after-vpentity-instrs 2>&1 -vplan-enable-new-cfg-merge-hir=0 | FileCheck %s
; RUN: opt -disable-output %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vplan-vec -vplan-enable-inmemory-entities -vplan-print-after-vpentity-instrs 2>&1 -vplan-enable-new-cfg-merge-hir=1 | FileCheck %s
; RUN: opt -disable-output %s -vplan-vec -vplan-print-after-vpentity-instrs 2>&1 | FileCheck %s

define i1 @fuseki(i32 %0, i8* nocapture %board, i64* nocapture %A, i64 %idxprom11.i, i8 %1, i1* %priv) local_unnamed_addr {
; CHECK-LABEL:  VPlan after insertion of VPEntities instructions:
; CHECK:        i64 [[VP__IND_INIT:%.*]] = induction-init{add} i64 0 i64 1
; CHECK:        i64 [[VP__IND_INIT_STEP:%.*]] = induction-init-step{add} i64 1
; All header PHIs should have same loop latch as second incoming block.
; CHECK-DAG:    i1 [[VP7:%.*]] = phi  [ i1 [[TMP2:.*]], [[PREHEADER:BB[0-9]+]] ],  [ i1 [[VP8:%.*]], [[LATCH:BB[0-9]+]] ]
; CHECK-DAG:    i64 [[VP_PRIV_IDX_HDR:%.*]] = phi  [ i64 -1, [[PREHEADER]] ],  [ i64 [[VP_PRIV_IDX_BB5:%.*]], [[LATCH]] ]
; CHECK-DAG:    i64 [[VP9:%.*]] = phi  [ i64 [[VP__IND_INIT]], [[PREHEADER]] ],  [ i64 [[VP10:%.*]], [[LATCH]] ]
;
entry:
  br label %for.body5.preheader.i

for.body5.preheader.i:                            ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE:TYPED"(i1* %priv, i1 zeroinitializer, i32 1) ]
  br label %for.body5.i

for.body5.i:                                      ; preds = %if.end33.i, %for.body5.preheader.i
  %indvars.iv.i = phi i64 [ 0, %for.body5.preheader.i ], [ %indvars.iv.next.i, %if.end33.i ]
  %2 = phi i1 [ true, %for.body5.preheader.i ], [ %4, %if.end33.i ]
  %idxprom.i = and i64 %indvars.iv.i, 1
  %arrayidx.i = getelementptr inbounds i8, i8* %board, i64 %idxprom.i
  %a.arrayidx = getelementptr inbounds i64, i64* %A, i64 %idxprom.i
  %3 = load i8, i8* %arrayidx.i, align 1
  %cmp14.not.i = icmp eq i8 %3, 0
  br i1 %cmp14.not.i, label %if.end.i, label %if.then.i

if.then.i:                                        ; preds = %for.body5.i
  store i64 42, i64* %a.arrayidx, align 4
  br label %if.end.i

if.end.i:                                         ; preds = %if.then.i, %for.body5.i
  %4 = phi i1 [ false, %if.then.i ], [ %2, %for.body5.i ]
  %cmp30.not.i = icmp eq i8 %3, %1
  br i1 %cmp30.not.i, label %if.end33.i, label %if.then32.i

if.then32.i:                                      ; preds = %if.end.i
  store i64 4242, i64* %a.arrayidx, align 4
  br label %if.end33.i

if.end33.i:                                       ; preds = %if.then32.i, %if.end.i
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %exitcond.not.i = icmp eq i64 %indvars.iv.i, %idxprom11.i
  br i1 %exitcond.not.i, label %for.inc54.i, label %for.body5.i

for.inc54.i:                                      ; preds = %if.end33.i
  %.lcssa = phi i1 [ %4, %if.end33.i ]
  store i1 %.lcssa, i1* %priv
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %set_symmetries.exit.loopexit

set_symmetries.exit.loopexit:                     ; preds = %for.inc54.i
  ret i1 %.lcssa
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
