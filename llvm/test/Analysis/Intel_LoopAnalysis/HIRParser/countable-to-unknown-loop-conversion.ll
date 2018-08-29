; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser | FileCheck %s
; RUN: opt < %s -passes=hir-ssa-deconstruction | opt -passes="print<hir-framework>" -hir-framework-debug=parser 2>&1 | FileCheck %s

; Verify that the countable inner loop is converted to unknown loop when parsing for the upper bound fails.


; CHECK: + DO i1 = 0, sext.i32.i64(%2) + -1 * sext.i32.i64(%1) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; CHECK: |   %zz.040.out = &((%zz.040)[0]);
; CHECK: |   %umax = (&((%scevgep42)[-1 * i1 + -1 * %1 + %3 + -1]) >u &((%R)[i1 + sext.i32.i64(%1) + sext.i32.i64(%indvars.iv)])) ? &((%scevgep42)[-1 * i1 + -1 * %1 + %3 + -1]) : &((%R)[i1 + sext.i32.i64(%1) + sext.i32.i64(%indvars.iv)]);
; CHECK: |   %umax48 = bitcast.double*.i8*(&((%umax)[0]));
; CHECK: |   %scevgep5051 = ptrtoint.i8*.i64(&((%umax48)[-8 * sext.i32.i64(%indvars.iv) + -1 * %R49 + -1]));
; CHECK: |   %sub3 = %3  +  -1 * i1 + -1 * %1 + -1;
; CHECK: |   if (i1 + sext.i32.i64(%1) > 0)
; CHECK: |   {
; CHECK: |      + UNKNOWN LOOP i2
; CHECK: |      |   <i2 = 0>
; CHECK: |      |   for.body9:
; CHECK: |      |   %15 = (i64*)(%R)[i2 + sext.i32.i64(%sub3)];
; CHECK: |      |   (i64*)(%zz.040.out)[i2] = %15;
; CHECK: |      |   if (&((%R)[i2 + sext.i32.i64(%sub3) + 1]) <u &((%R)[i1 + sext.i32.i64(%1) + sext.i32.i64(%sub3)]))
; CHECK: |      |   {
; CHECK: |      |      <i2 = i2 + 1>
; CHECK: |      |      goto for.body9;
; CHECK: |      |   }
; CHECK: |      + END LOOP
; CHECK: |
; CHECK: |      %zz.040 = &((%zz.040.out)[(%scevgep5051 /u 8) + 1]);
; CHECK: |   }
; CHECK: |   %indvars.iv = -1 * i1 + -1 * %1 + %3 + -2;
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @mgcv_pbsi(double* readonly %R, i32* nocapture readonly %r, i32* nocapture readnone %nt) local_unnamed_addr #0 {
entry:
  %call = tail call i32 (i32, ...) bitcast (i32 (...)* @R_chk_calloc to i32 (i32, ...)*)(i32 0) #2
  %conv = sext i32 %call to i64
  %0 = inttoptr i64 %conv to i32*
  %1 = load i32, i32* %0, align 4
  %arrayidx1 = getelementptr inbounds i32, i32* %0, i64 1
  %2 = load i32, i32* %arrayidx1, align 4
  %cmp39 = icmp slt i32 %1, %2
  br i1 %cmp39, label %for.body.lr.ph, label %for.end12

for.body.lr.ph:                                   ; preds = %entry
  %R49 = ptrtoint double* %R to i64
  %3 = load i32, i32* %r, align 4
  %scevgep42 = getelementptr double, double* %R, i64 1
  %4 = add i32 %3, -1
  %5 = sub i32 %4, %1
  %6 = sext i32 %1 to i64
  %scevgep44 = getelementptr double, double* %R, i64 %6
  %7 = xor i64 %R49, -1
  %8 = sext i32 %2 to i64
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc11
  %indvars.iv54 = phi i64 [ %6, %for.body.lr.ph ], [ %indvars.iv.next55, %for.inc11 ]
  %indvars.iv45 = phi double* [ %scevgep44, %for.body.lr.ph ], [ %scevgep46, %for.inc11 ]
  %indvars.iv = phi i32 [ %5, %for.body.lr.ph ], [ %indvars.iv.next, %for.inc11 ]
  %zz.040 = phi double* [ undef, %for.body.lr.ph ], [ %zz.1.lcssa, %for.inc11 ]
  %9 = sext i32 %indvars.iv to i64
  %scevgep43 = getelementptr double, double* %scevgep42, i64 %9
  %scevgep47 = getelementptr double, double* %indvars.iv45, i64 %9
  %10 = icmp ugt double* %scevgep43, %scevgep47
  %umax = select i1 %10, double* %scevgep43, double* %scevgep47
  %umax48 = bitcast double* %umax to i8*
  %uglygep = getelementptr i8, i8* %umax48, i64 %7
  %11 = mul nsw i64 %9, -8
  %scevgep50 = getelementptr i8, i8* %uglygep, i64 %11
  %scevgep5051 = ptrtoint i8* %scevgep50 to i64
  %12 = lshr i64 %scevgep5051, 3
  %13 = trunc i64 %indvars.iv54 to i32
  %sub = xor i32 %13, -1
  %sub3 = add i32 %3, %sub
  %idx.ext = sext i32 %sub3 to i64
  %add.ptr = getelementptr inbounds double, double* %R, i64 %idx.ext
  %add.ptr5 = getelementptr inbounds double, double* %add.ptr, i64 %indvars.iv54
  %cmp736 = icmp sgt i64 %indvars.iv54, 0
  br i1 %cmp736, label %for.body9.lr.ph, label %for.inc11

for.body9.lr.ph:                                  ; preds = %for.body
  %scevgep = getelementptr double, double* %zz.040, i64 1
  br label %for.body9

for.body9:                                        ; preds = %for.body9.lr.ph, %for.body9
  %zz.138 = phi double* [ %zz.040, %for.body9.lr.ph ], [ %incdec.ptr10, %for.body9 ]
  %rr.037 = phi double* [ %add.ptr, %for.body9.lr.ph ], [ %incdec.ptr, %for.body9 ]
  %14 = bitcast double* %rr.037 to i64*
  %15 = load i64, i64* %14, align 8
  %16 = bitcast double* %zz.138 to i64*
  store i64 %15, i64* %16, align 8
  %incdec.ptr = getelementptr inbounds double, double* %rr.037, i64 1
  %incdec.ptr10 = getelementptr inbounds double, double* %zz.138, i64 1
  %cmp7 = icmp ult double* %incdec.ptr, %add.ptr5
  br i1 %cmp7, label %for.body9, label %for.inc11.loopexit

for.inc11.loopexit:                               ; preds = %for.body9
  %scevgep52 = getelementptr double, double* %scevgep, i64 %12
  br label %for.inc11

for.inc11:                                        ; preds = %for.inc11.loopexit, %for.body
  %zz.1.lcssa = phi double* [ %zz.040, %for.body ], [ %scevgep52, %for.inc11.loopexit ]
  %indvars.iv.next55 = add nsw i64 %indvars.iv54, 1
  %cmp = icmp slt i64 %indvars.iv.next55, %8
  %indvars.iv.next = add i32 %indvars.iv, -1
  %scevgep46 = getelementptr double, double* %indvars.iv45, i64 1
  br i1 %cmp, label %for.body, label %for.end12.loopexit

for.end12.loopexit:                               ; preds = %for.inc11
  br label %for.end12

for.end12:                                        ; preds = %for.end12.loopexit, %entry
  ret void
}

declare i32 @R_chk_calloc(...) local_unnamed_addr
