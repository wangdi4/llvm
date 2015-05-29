; RUN: opt < %s -instcombine -S | FileCheck %s -check-prefix=SUBSW

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; the source code:
;#define T1 signed short
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
;#pragma clang loop vectorize_width(8)
;  for (i=0;i<N;i++){
;    b[i] = SAT2SI16(a[i]-b[i]);
;  }
;}

; SUBSW-LABEL: kernel
; SUBSW: ssat.sub.v8i16
define void @kernel(i16* nocapture readonly %a, i16* noalias nocapture %b, i32 %N) #0 {
entry:
  %cmp59 = icmp sgt i32 %N, 0
  br i1 %cmp59, label %for.body.preheader, label %for.end

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
  %broadcast.splatinsert62 = insertelement <8 x i32> undef, i32 %N, i32 0
  %broadcast.splat63 = shufflevector <8 x i32> %broadcast.splatinsert62, <8 x i32> undef, <8 x i32> zeroinitializer
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %vector.ph
  %index = phi i64 [ 0, %vector.ph ], [ %index.next, %vector.body ]
  %broadcast.splatinsert = insertelement <8 x i64> undef, i64 %index, i32 0
  %broadcast.splat = shufflevector <8 x i64> %broadcast.splatinsert, <8 x i64> undef, <8 x i32> zeroinitializer
  %induction = add <8 x i64> %broadcast.splat, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>
  %3 = extractelement <8 x i64> %induction, i32 0
  %4 = getelementptr inbounds i16, i16* %a, i64 %3
  %5 = insertelement <8 x i16*> undef, i16* %4, i32 0
  %6 = extractelement <8 x i64> %induction, i32 1
  %7 = getelementptr inbounds i16, i16* %a, i64 %6
  %8 = insertelement <8 x i16*> %5, i16* %7, i32 1
  %9 = extractelement <8 x i64> %induction, i32 2
  %10 = getelementptr inbounds i16, i16* %a, i64 %9
  %11 = insertelement <8 x i16*> %8, i16* %10, i32 2
  %12 = extractelement <8 x i64> %induction, i32 3
  %13 = getelementptr inbounds i16, i16* %a, i64 %12
  %14 = insertelement <8 x i16*> %11, i16* %13, i32 3
  %15 = extractelement <8 x i64> %induction, i32 4
  %16 = getelementptr inbounds i16, i16* %a, i64 %15
  %17 = insertelement <8 x i16*> %14, i16* %16, i32 4
  %18 = extractelement <8 x i64> %induction, i32 5
  %19 = getelementptr inbounds i16, i16* %a, i64 %18
  %20 = insertelement <8 x i16*> %17, i16* %19, i32 5
  %21 = extractelement <8 x i64> %induction, i32 6
  %22 = getelementptr inbounds i16, i16* %a, i64 %21
  %23 = insertelement <8 x i16*> %20, i16* %22, i32 6
  %24 = extractelement <8 x i64> %induction, i32 7
  %25 = getelementptr inbounds i16, i16* %a, i64 %24
  %26 = insertelement <8 x i16*> %23, i16* %25, i32 7
  %27 = getelementptr i16, i16* %4, i32 0
  %28 = bitcast i16* %27 to <8 x i16>*
  %wide.load = load <8 x i16>, <8 x i16>* %28, align 2
  %29 = sext <8 x i16> %wide.load to <8 x i32>
  %30 = getelementptr inbounds i16, i16* %b, i64 %3
  %31 = insertelement <8 x i16*> undef, i16* %30, i32 0
  %32 = getelementptr inbounds i16, i16* %b, i64 %6
  %33 = insertelement <8 x i16*> %31, i16* %32, i32 1
  %34 = getelementptr inbounds i16, i16* %b, i64 %9
  %35 = insertelement <8 x i16*> %33, i16* %34, i32 2
  %36 = getelementptr inbounds i16, i16* %b, i64 %12
  %37 = insertelement <8 x i16*> %35, i16* %36, i32 3
  %38 = getelementptr inbounds i16, i16* %b, i64 %15
  %39 = insertelement <8 x i16*> %37, i16* %38, i32 4
  %40 = getelementptr inbounds i16, i16* %b, i64 %18
  %41 = insertelement <8 x i16*> %39, i16* %40, i32 5
  %42 = getelementptr inbounds i16, i16* %b, i64 %21
  %43 = insertelement <8 x i16*> %41, i16* %42, i32 6
  %44 = getelementptr inbounds i16, i16* %b, i64 %24
  %45 = insertelement <8 x i16*> %43, i16* %44, i32 7
  %46 = getelementptr i16, i16* %30, i32 0
  %47 = bitcast i16* %46 to <8 x i16>*
  %wide.load61 = load <8 x i16>, <8 x i16>* %47, align 2
  %48 = sext <8 x i16> %wide.load61 to <8 x i32>
  %49 = sub nsw <8 x i32> %48, %29
  %50 = icmp sgt <8 x i32> %49, <i32 32767, i32 32767, i32 32767, i32 32767, i32 32767, i32 32767, i32 32767, i32 32767>
  %51 = extractelement <8 x i1> %50, i32 0
  %52 = select <8 x i1> %50, <8 x i32> <i32 32767, i32 32767, i32 32767, i32 32767, i32 32767, i32 32767, i32 32767, i32 32767>, <8 x i32> %49
  %53 = icmp slt <8 x i32> %52, <i32 -32768, i32 -32768, i32 -32768, i32 -32768, i32 -32768, i32 -32768, i32 -32768, i32 -32768>
  %54 = trunc <8 x i32> %52 to <8 x i16>
  %55 = extractelement <8 x i1> %53, i32 0
  %56 = select <8 x i1> %53, <8 x i16> <i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768, i16 -32768>, <8 x i16> %54
  %57 = bitcast i16* %46 to <8 x i16>*
  store <8 x i16> %56, <8 x i16>* %57, align 2
  %58 = add nuw nsw <8 x i64> %induction, <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
  %59 = trunc <8 x i64> %58 to <8 x i32>
  %60 = icmp eq <8 x i32> %59, %broadcast.splat63
  %index.next = add i64 %index, 8
  %61 = icmp eq i64 %index.next, %end.idx.rnd.down
  br i1 %61, label %middle.block, label %vector.body

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
  %arrayidx = getelementptr inbounds i16, i16* %a, i64 %indvars.iv
  %62 = load i16, i16* %arrayidx, align 2
  %conv = sext i16 %62 to i32
  %arrayidx2 = getelementptr inbounds i16, i16* %b, i64 %indvars.iv
  %63 = load i16, i16* %arrayidx2, align 2
  %conv3 = sext i16 %63 to i32
  %add = sub nsw i32 %conv3, %conv
  %cmp4 = icmp sgt i32 %add, 32767
  %.add = select i1 %cmp4, i32 32767, i32 %add
  %cmp13 = icmp slt i32 %.add, -32768
  %64 = trunc i32 %.add to i16
  %conv39 = select i1 %cmp13, i16 -32768, i16 %64
  store i16 %conv39, i16* %arrayidx2, align 2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %N
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %middle.block, %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

