; RUN: opt -passes="hir-ssa-deconstruction,print<hir-framework>" -disable-output < %s 2>&1 | FileCheck %s

; Check that we are able to compute the max trip count estimate of while.body.i by tracing from %cptr.1.i79 all the way back to %1 to get to the [12 x i8] array type.

; CHECK: + DO i1 = 0, %inc.i.lcssa + -1, 1   <DO_LOOP>  <MAX_TC_EST = 12>
; CHECK: |   %hir.de.ssa.copy0.out = &((@dot_dec.bufs)[0][i1 + 16 * %0]);
; CHECK: |   %5 = (%incdec.ptr.i.lcssa)[-1 * i1 + -1];
; CHECK: |   %incdec.ptr3.i = &((@dot_dec.bufs)[0][i1 + 16 * %0 + 1]);
; CHECK: |   (@dot_dec.bufs)[0][i1 + 16 * %0] = %5;
; CHECK: + END LOOP


target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"

@dot_dec.nxt = internal unnamed_addr global i32 0, align 4
@dot_dec.bufs = internal global [256 x i8] zeroinitializer, align 1
@.str.5 = private unnamed_addr constant [11 x i8] c"0123456789\00", align 1

define ptr @dot_dec(i32 %ip) local_unnamed_addr {
entry:
  %tmpbuf.i = alloca [12 x i8], align 1
  %0 = load i32, ptr @dot_dec.nxt, align 4
  %mul = shl nsw i32 %0, 4
  %add.ptr = getelementptr inbounds [256 x i8], ptr @dot_dec.bufs, i32 0, i32 %mul
  %cmp = icmp eq i32 %0, 15
  %add = add nsw i32 %0, 1
  %.add = select i1 %cmp, i32 0, i32 %add
  store i32 %.add, ptr @dot_dec.nxt, align 4
  %shr = lshr i32 %ip, 24
  %1 = getelementptr inbounds [12 x i8], ptr %tmpbuf.i, i32 0, i32 0
  br label %do.body.i

do.body.i:                                        ; preds = %do.body.i, %entry
  %x.addr.0.i = phi i32 [ %shr, %entry ], [ %div.i, %do.body.i ]
  %n.0.i = phi i32 [ 0, %entry ], [ %inc.i, %do.body.i ]
  %cptr.0.i = phi ptr [ %1, %entry ], [ %incdec.ptr.i, %do.body.i ]
  %rem.i = urem i32 %x.addr.0.i, 10
  %arrayidx.i = getelementptr inbounds [11 x i8], ptr @.str.5, i32 0, i32 %rem.i
  %2 = load i8, ptr %arrayidx.i, align 1
  %incdec.ptr.i = getelementptr inbounds i8, ptr %cptr.0.i, i32 1
  store i8 %2, ptr %cptr.0.i, align 1
  %inc.i = add nuw nsw i32 %n.0.i, 1
  %div.i = udiv i32 %x.addr.0.i, 10
  %3 = icmp ugt i32 %x.addr.0.i, 9
  br i1 %3, label %do.body.i, label %while.body.i.preheader

while.body.i.preheader:                           ; preds = %do.body.i
  %incdec.ptr.i.lcssa = phi ptr [ %incdec.ptr.i, %do.body.i ]
  %inc.i.lcssa = phi i32 [ %inc.i, %do.body.i ]
  br label %while.body.i

while.body.i:                                     ; preds = %while.body.i.preheader, %while.body.i
  %4 = phi ptr [ %incdec.ptr3.i, %while.body.i ], [ %add.ptr, %while.body.i.preheader ]
  %cptr.1.i79 = phi ptr [ %incdec.ptr2.i, %while.body.i ], [ %incdec.ptr.i.lcssa, %while.body.i.preheader ]
  %n.1.i78 = phi i32 [ %dec.i, %while.body.i ], [ %inc.i.lcssa, %while.body.i.preheader ]
  %dec.i = add nsw i32 %n.1.i78, -1
  %incdec.ptr2.i = getelementptr inbounds i8, ptr %cptr.1.i79, i32 -1
  %5 = load i8, ptr %incdec.ptr2.i, align 1
  %incdec.ptr3.i = getelementptr inbounds i8, ptr %4, i32 1
  store i8 %5, ptr %4, align 1
  %tobool1.i = icmp eq i32 %dec.i, 0
  br i1 %tobool1.i, label %prti.exit, label %while.body.i

prti.exit:                                        ; preds = %while.body.i
  %.lcssa88 = phi ptr [ %4, %while.body.i ]
  ret ptr %incdec.ptr3.i
}

