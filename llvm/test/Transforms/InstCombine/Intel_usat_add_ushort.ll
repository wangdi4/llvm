; RUN: opt < %s -instcombine -S | FileCheck %s -check-prefix=ADDUSW
; XFAIL: *

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; the source code:
;#define T1 unsigned short
;#define T2 unsigned short
;#define MIN(x,y) ((x)>(y) ? (y) : (x))
;#define MAX(x,y) ((x)<(y) ? (y) : (x))
;#define MAX_I16 65535
;#define MIN_I16 0
;#define SAT2I16(x) (MAX(MIN(x,MAX_I16), MIN_I16))
;
;void kernel(T1 *a, T2 *b, int N){
;  int i;
;#pragma clang loop vectorize (enable)
;#pragma clang loop vectorize_width (8)
;  for (i=0;i<N;i++){
;    b[i] = SAT2I16(a[i]+b[i]);
;  }
;}

; ADDUSW-LABEL: kernel
; ADDUSW: usat.add.v8i16
define void @kernel(i16* nocapture readonly %a, i16* nocapture %b, i32 %N) #0 {
entry:
  %b61 = bitcast i16* %b to i8*
  %a63 = bitcast i16* %a to i8*
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
  %3 = add i32 %N, -1
  %4 = zext i32 %3 to i64
  %scevgep = getelementptr i16, i16* %b, i64 %4
  %scevgep62 = bitcast i16* %scevgep to i8*
  %scevgep64 = getelementptr i16, i16* %a, i64 %4
  %scevgep6465 = bitcast i16* %scevgep64 to i8*
  br i1 %cmp.zero, label %middle.block, label %vector.memcheck

vector.memcheck:                                  ; preds = %overflow.checked
  %bound0 = icmp ule i8* %b61, %scevgep6465
  %bound1 = icmp ule i8* %a63, %scevgep62
  %found.conflict = and i1 %bound0, %bound1
  %memcheck.conflict = and i1 %found.conflict, true
  br i1 %memcheck.conflict, label %middle.block, label %vector.ph

vector.ph:                                        ; preds = %vector.memcheck
  %broadcast.splatinsert67 = insertelement <8 x i32> undef, i32 %N, i32 0
  %broadcast.splat68 = shufflevector <8 x i32> %broadcast.splatinsert67, <8 x i32> undef, <8 x i32> zeroinitializer
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %vector.ph
  %index = phi i64 [ 0, %vector.ph ], [ %index.next, %vector.body ]
  %broadcast.splatinsert = insertelement <8 x i64> undef, i64 %index, i32 0
  %broadcast.splat = shufflevector <8 x i64> %broadcast.splatinsert, <8 x i64> undef, <8 x i32> zeroinitializer
  %induction = add <8 x i64> %broadcast.splat, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>
  %5 = extractelement <8 x i64> %induction, i32 0
  %6 = getelementptr inbounds i16, i16* %a, i64 %5
  %7 = insertelement <8 x i16*> undef, i16* %6, i32 0
  %8 = extractelement <8 x i64> %induction, i32 1
  %9 = getelementptr inbounds i16, i16* %a, i64 %8
  %10 = insertelement <8 x i16*> %7, i16* %9, i32 1
  %11 = extractelement <8 x i64> %induction, i32 2
  %12 = getelementptr inbounds i16, i16* %a, i64 %11
  %13 = insertelement <8 x i16*> %10, i16* %12, i32 2
  %14 = extractelement <8 x i64> %induction, i32 3
  %15 = getelementptr inbounds i16, i16* %a, i64 %14
  %16 = insertelement <8 x i16*> %13, i16* %15, i32 3
  %17 = extractelement <8 x i64> %induction, i32 4
  %18 = getelementptr inbounds i16, i16* %a, i64 %17
  %19 = insertelement <8 x i16*> %16, i16* %18, i32 4
  %20 = extractelement <8 x i64> %induction, i32 5
  %21 = getelementptr inbounds i16, i16* %a, i64 %20
  %22 = insertelement <8 x i16*> %19, i16* %21, i32 5
  %23 = extractelement <8 x i64> %induction, i32 6
  %24 = getelementptr inbounds i16, i16* %a, i64 %23
  %25 = insertelement <8 x i16*> %22, i16* %24, i32 6
  %26 = extractelement <8 x i64> %induction, i32 7
  %27 = getelementptr inbounds i16, i16* %a, i64 %26
  %28 = insertelement <8 x i16*> %25, i16* %27, i32 7
  %29 = getelementptr i16, i16* %6, i32 0
  %30 = bitcast i16* %29 to <8 x i16>*
  %wide.load = load <8 x i16>, <8 x i16>* %30, align 2
  %31 = zext <8 x i16> %wide.load to <8 x i32>
  %32 = getelementptr inbounds i16, i16* %b, i64 %5
  %33 = insertelement <8 x i16*> undef, i16* %32, i32 0
  %34 = getelementptr inbounds i16, i16* %b, i64 %8
  %35 = insertelement <8 x i16*> %33, i16* %34, i32 1
  %36 = getelementptr inbounds i16, i16* %b, i64 %11
  %37 = insertelement <8 x i16*> %35, i16* %36, i32 2
  %38 = getelementptr inbounds i16, i16* %b, i64 %14
  %39 = insertelement <8 x i16*> %37, i16* %38, i32 3
  %40 = getelementptr inbounds i16, i16* %b, i64 %17
  %41 = insertelement <8 x i16*> %39, i16* %40, i32 4
  %42 = getelementptr inbounds i16, i16* %b, i64 %20
  %43 = insertelement <8 x i16*> %41, i16* %42, i32 5
  %44 = getelementptr inbounds i16, i16* %b, i64 %23
  %45 = insertelement <8 x i16*> %43, i16* %44, i32 6
  %46 = getelementptr inbounds i16, i16* %b, i64 %26
  %47 = insertelement <8 x i16*> %45, i16* %46, i32 7
  %48 = getelementptr i16, i16* %32, i32 0
  %49 = bitcast i16* %48 to <8 x i16>*
  %wide.load66 = load <8 x i16>, <8 x i16>* %49, align 2
  %50 = zext <8 x i16> %wide.load66 to <8 x i32>
  %51 = add nuw nsw <8 x i32> %50, %31
  %52 = icmp sgt <8 x i32> %51, <i32 65535, i32 65535, i32 65535, i32 65535, i32 65535, i32 65535, i32 65535, i32 65535>
  %53 = trunc <8 x i32> %51 to <8 x i16>
  %54 = extractelement <8 x i1> %52, i32 0
  %55 = select <8 x i1> %52, <8 x i16> <i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1>, <8 x i16> %53
  %56 = bitcast i16* %48 to <8 x i16>*
  store <8 x i16> %55, <8 x i16>* %56, align 2
  %57 = add nuw nsw <8 x i64> %induction, <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
  %58 = trunc <8 x i64> %57 to <8 x i32>
  %59 = icmp eq <8 x i32> %58, %broadcast.splat68
  %index.next = add i64 %index, 8
  %60 = icmp eq i64 %index.next, %end.idx.rnd.down
  br i1 %60, label %middle.block, label %vector.body

middle.block:                                     ; preds = %vector.body, %vector.memcheck, %overflow.checked
  %resume.val = phi i64 [ 0, %overflow.checked ], [ 0, %vector.memcheck ], [ %end.idx.rnd.down, %vector.body ]
  %trunc.resume.val = phi i64 [ 0, %overflow.checked ], [ 0, %vector.memcheck ], [ %end.idx.rnd.down, %vector.body ]
  %cmp.n = icmp eq i64 %end.idx, %resume.val
  br i1 %cmp.n, label %for.end.loopexit, label %scalar.ph

scalar.ph:                                        ; preds = %middle.block, %for.body.preheader
  %bc.resume.val = phi i64 [ %resume.val, %middle.block ], [ 0, %for.body.preheader ]
  %bc.trunc.resume.val = phi i64 [ %trunc.resume.val, %middle.block ], [ 0, %for.body.preheader ]
  br label %for.body

for.body:                                         ; preds = %scalar.ph, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ %bc.trunc.resume.val, %scalar.ph ]
  %arrayidx = getelementptr inbounds i16, i16* %a, i64 %indvars.iv
  %61 = load i16, i16* %arrayidx, align 2
  %conv = zext i16 %61 to i32
  %arrayidx2 = getelementptr inbounds i16, i16* %b, i64 %indvars.iv
  %62 = load i16, i16* %arrayidx2, align 2
  %conv3 = zext i16 %62 to i32
  %add = add nuw nsw i32 %conv3, %conv
  %cmp24 = icmp sgt i32 %add, 65535
  %63 = trunc i32 %add to i16
  %conv39 = select i1 %cmp24, i16 -1, i16 %63
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

