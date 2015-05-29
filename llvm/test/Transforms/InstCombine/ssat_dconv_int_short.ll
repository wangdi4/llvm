; RUN: opt < %s -instcombine -S | FileCheck %s -check-prefix=PACKSSDW

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; the source code:
;#define T1 signed int
;#define T2 signed short
;#define MIN(x,y) ((x)>(y) ? (y) : (x))
;#define MAX(x,y) ((x)<(y) ? (y) : (x))
;#define MAX_SI16 32767
;#define MIN_SI16 -32768
;#define SAT2SI16(x) (MAX(MIN(x,MAX_SI16), MIN_SI16))
;
;void kernel(T1 *a, T2 *restrict b, int N){
;  int i;
;#pragma clang loop vectorize (enable)
;#pragma clang loop vectorize_width (8)
;  for (i=0;i<N;i++){
;    b[i] = SAT2SI16(a[i]);
;  }
;}

; PACKSSDW-LABEL: kernel
; PACKSSDW: ssat.dcnv.v8i16
define void @kernel(i32* nocapture readonly %a, i16* noalias nocapture %b, i32 %N) #0 {
entry:
  %cmp30 = icmp sgt i32 %N, 0
  br i1 %cmp30, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %0 = add i32 %N, -1
  %1 = zext i32 %0 to i64
  %backedge.overflow = icmp eq i64 %1, -1
  %overflow.check.anchor = add i64 0, 0
  br i1 %backedge.overflow, label %scalar.ph, label %overflow.checked

overflow.checked:                                 ; preds = %for.body.preheader
  %2 = add i64 %1, 1
  %end.idx = add i64 %2, 0
  %n.mod.vf = urem i64 %2, 8
  %n.vec = sub i64 %2, %n.mod.vf
  %end.idx.rnd.down = add i64 %n.vec, 0
  %cmp.zero = icmp eq i64 %end.idx.rnd.down, 0
  br i1 %cmp.zero, label %middle.block, label %vector.ph

vector.ph:                                        ; preds = %overflow.checked
  %broadcast.splatinsert32 = insertelement <8 x i32> undef, i32 %N, i32 0
  %broadcast.splat33 = shufflevector <8 x i32> %broadcast.splatinsert32, <8 x i32> undef, <8 x i32> zeroinitializer
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %vector.ph
  %index = phi i64 [ 0, %vector.ph ], [ %index.next, %vector.body ]
  %broadcast.splatinsert = insertelement <8 x i64> undef, i64 %index, i32 0
  %broadcast.splat = shufflevector <8 x i64> %broadcast.splatinsert, <8 x i64> undef, <8 x i32> zeroinitializer
  %induction = add <8 x i64> %broadcast.splat, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>
  %3 = extractelement <8 x i64> %induction, i32 0
  %4 = getelementptr inbounds i32, i32* %a, i64 %3
  %5 = insertelement <8 x i32*> undef, i32* %4, i32 0
  %6 = extractelement <8 x i64> %induction, i32 1
  %7 = getelementptr inbounds i32, i32* %a, i64 %6
  %8 = insertelement <8 x i32*> %5, i32* %7, i32 1
  %9 = extractelement <8 x i64> %induction, i32 2
  %10 = getelementptr inbounds i32, i32* %a, i64 %9
  %11 = insertelement <8 x i32*> %8, i32* %10, i32 2
  %12 = extractelement <8 x i64> %induction, i32 3
  %13 = getelementptr inbounds i32, i32* %a, i64 %12
  %14 = insertelement <8 x i32*> %11, i32* %13, i32 3
  %15 = extractelement <8 x i64> %induction, i32 4
  %16 = getelementptr inbounds i32, i32* %a, i64 %15
  %17 = insertelement <8 x i32*> %14, i32* %16, i32 4
  %18 = extractelement <8 x i64> %induction, i32 5
  %19 = getelementptr inbounds i32, i32* %a, i64 %18
  %20 = insertelement <8 x i32*> %17, i32* %19, i32 5
  %21 = extractelement <8 x i64> %induction, i32 6
  %22 = getelementptr inbounds i32, i32* %a, i64 %21
  %23 = insertelement <8 x i32*> %20, i32* %22, i32 6
  %24 = extractelement <8 x i64> %induction, i32 7
  %25 = getelementptr inbounds i32, i32* %a, i64 %24
  %26 = insertelement <8 x i32*> %23, i32* %25, i32 7
  %27 = getelementptr i32, i32* %4, i32 0
  %28 = bitcast i32* %27 to <8 x i32>*
  %wide.load = load <8 x i32>, <8 x i32>* %28, align 4
  %29 = icmp sgt <8 x i32> %wide.load, <i32 32767, i32 32767, i32 32767, i32 32767, i32 32767, i32 32767, i32 32767, i32 32767>
  %30 = extractelement <8 x i1> %29, i32 0
  %31 = select <8 x i1> %29, <8 x i32> <i32 32767, i32 32767, i32 32767, i32 32767, i32 32767, i32 32767, i32 32767, i32 32767>, <8 x i32> %wide.load
  %32 = icmp slt <8 x i32> %31, <i32 -32768, i32 -32768, i32 -32768, i32 -32768, i32 -32768, i32 -32768, i32 -32768, i32 -32768>
  %33 = trunc <8 x i32> %31 to <8 x i16>
  %34 = extractelement <8 x i1> %32, i32 0
  %35 = select <8 x i1> %32, <8 x i16> <i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768>, <8 x i16> %33
  %36 = getelementptr inbounds i16, i16* %b, i64 %3
  %37 = insertelement <8 x i16*> undef, i16* %36, i32 0
  %38 = getelementptr inbounds i16, i16* %b, i64 %6
  %39 = insertelement <8 x i16*> %37, i16* %38, i32 1
  %40 = getelementptr inbounds i16, i16* %b, i64 %9
  %41 = insertelement <8 x i16*> %39, i16* %40, i32 2
  %42 = getelementptr inbounds i16, i16* %b, i64 %12
  %43 = insertelement <8 x i16*> %41, i16* %42, i32 3
  %44 = getelementptr inbounds i16, i16* %b, i64 %15
  %45 = insertelement <8 x i16*> %43, i16* %44, i32 4
  %46 = getelementptr inbounds i16, i16* %b, i64 %18
  %47 = insertelement <8 x i16*> %45, i16* %46, i32 5
  %48 = getelementptr inbounds i16, i16* %b, i64 %21
  %49 = insertelement <8 x i16*> %47, i16* %48, i32 6
  %50 = getelementptr inbounds i16, i16* %b, i64 %24
  %51 = insertelement <8 x i16*> %49, i16* %50, i32 7
  %52 = getelementptr i16, i16* %36, i32 0
  %53 = bitcast i16* %52 to <8 x i16>*
  store <8 x i16> %35, <8 x i16>* %53, align 2
  %54 = add nuw nsw <8 x i64> %induction, <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
  %55 = trunc <8 x i64> %54 to <8 x i32>
  %56 = icmp eq <8 x i32> %55, %broadcast.splat33
  %index.next = add i64 %index, 8
  %57 = icmp eq i64 %index.next, %end.idx.rnd.down
  br i1 %57, label %middle.block, label %vector.body

middle.block:                                     ; preds = %vector.body, %overflow.checked
  %resume.val = phi i64 [ 0, %overflow.checked ], [ %end.idx.rnd.down, %vector.body ]
  %trunc.resume.val = phi i64 [ 0, %overflow.checked ], [ %end.idx.rnd.down, %vector.body ]
  %cmp.n = icmp eq i64 %end.idx, %resume.val
  br i1 %cmp.n, label %for.end.loopexit, label %scalar.ph

scalar.ph:                                        ; preds = %middle.block, %for.body.preheader
  %bc.resume.val = phi i64 [ %resume.val, %middle.block ], [ 0, %for.body.preheader ]
  %bc.trunc.resume.val = phi i64 [ %trunc.resume.val, %middle.block ], [ 0, %for.body.preheader ]
  br label %for.body

for.body:                                         ; preds = %scalar.ph, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ %bc.trunc.resume.val, %scalar.ph ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %58 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp sgt i32 %58, 32767
  %. = select i1 %cmp1, i32 32767, i32 %58
  %cmp4 = icmp slt i32 %., -32768
  %59 = trunc i32 %. to i16
  %conv = select i1 %cmp4, i16 -32768, i16 %59
  %arrayidx19 = getelementptr inbounds i16, i16* %b, i64 %indvars.iv
  store i16 %conv, i16* %arrayidx19, align 2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %N
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %middle.block, %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

