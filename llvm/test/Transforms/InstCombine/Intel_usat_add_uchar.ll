; RUN: opt < %s -instcombine -S | FileCheck %s -check-prefix=ADDUSB
; XFAIL: *

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; the source code:
;#define T1 unsigned char
;#define T2 unsigned char
;#define MIN(x,y) ((x)>(y) ? (y) : (x))
;#define MAX(x,y) ((x)<(y) ? (y) : (x))
;#define MAX_I8 255
;#define MIN_I8 0
;#define SAT2I8(x) (MAX(MIN(x,MAX_I8), MIN_I8))
;
;void kernel(T1 *a, T2 *b, int N){
;  int i;
;#pragma clang loop vectorize (enable)
;#pragma clang loop vectorize_width (16)
;  for (i=0;i<N;i++){
;    b[i] = SAT2I8(a[i]+b[i]);
;  }
;}

; ADDUSB-LABEL: kernel
; ADDUSB: usat.add.v16i8
define void @kernel(i8* nocapture readonly %a, i8* nocapture %b, i32 %N) #0 {
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
  %n.mod.vf = urem i64 %2, 16
  %n.vec = sub i64 %2, %n.mod.vf
  %end.idx.rnd.down = add i64 %n.vec, 0
  %cmp.zero = icmp eq i64 %end.idx.rnd.down, 0
  %3 = add i32 %N, -1
  %4 = zext i32 %3 to i64
  %scevgep = getelementptr i8, i8* %b, i64 %4
  %scevgep61 = getelementptr i8, i8* %a, i64 %4
  br i1 %cmp.zero, label %middle.block, label %vector.memcheck

vector.memcheck:                                  ; preds = %overflow.checked
  %bound0 = icmp ule i8* %b, %scevgep61
  %bound1 = icmp ule i8* %a, %scevgep
  %found.conflict = and i1 %bound0, %bound1
  %memcheck.conflict = and i1 %found.conflict, true
  br i1 %memcheck.conflict, label %middle.block, label %vector.ph

vector.ph:                                        ; preds = %vector.memcheck
  %broadcast.splatinsert63 = insertelement <16 x i32> undef, i32 %N, i32 0
  %broadcast.splat64 = shufflevector <16 x i32> %broadcast.splatinsert63, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %vector.ph
  %index = phi i64 [ 0, %vector.ph ], [ %index.next, %vector.body ]
  %broadcast.splatinsert = insertelement <16 x i64> undef, i64 %index, i32 0
  %broadcast.splat = shufflevector <16 x i64> %broadcast.splatinsert, <16 x i64> undef, <16 x i32> zeroinitializer
  %induction = add <16 x i64> %broadcast.splat, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %5 = extractelement <16 x i64> %induction, i32 0
  %6 = getelementptr inbounds i8, i8* %a, i64 %5
  %7 = insertelement <16 x i8*> undef, i8* %6, i32 0
  %8 = extractelement <16 x i64> %induction, i32 1
  %9 = getelementptr inbounds i8, i8* %a, i64 %8
  %10 = insertelement <16 x i8*> %7, i8* %9, i32 1
  %11 = extractelement <16 x i64> %induction, i32 2
  %12 = getelementptr inbounds i8, i8* %a, i64 %11
  %13 = insertelement <16 x i8*> %10, i8* %12, i32 2
  %14 = extractelement <16 x i64> %induction, i32 3
  %15 = getelementptr inbounds i8, i8* %a, i64 %14
  %16 = insertelement <16 x i8*> %13, i8* %15, i32 3
  %17 = extractelement <16 x i64> %induction, i32 4
  %18 = getelementptr inbounds i8, i8* %a, i64 %17
  %19 = insertelement <16 x i8*> %16, i8* %18, i32 4
  %20 = extractelement <16 x i64> %induction, i32 5
  %21 = getelementptr inbounds i8, i8* %a, i64 %20
  %22 = insertelement <16 x i8*> %19, i8* %21, i32 5
  %23 = extractelement <16 x i64> %induction, i32 6
  %24 = getelementptr inbounds i8, i8* %a, i64 %23
  %25 = insertelement <16 x i8*> %22, i8* %24, i32 6
  %26 = extractelement <16 x i64> %induction, i32 7
  %27 = getelementptr inbounds i8, i8* %a, i64 %26
  %28 = insertelement <16 x i8*> %25, i8* %27, i32 7
  %29 = extractelement <16 x i64> %induction, i32 8
  %30 = getelementptr inbounds i8, i8* %a, i64 %29
  %31 = insertelement <16 x i8*> %28, i8* %30, i32 8
  %32 = extractelement <16 x i64> %induction, i32 9
  %33 = getelementptr inbounds i8, i8* %a, i64 %32
  %34 = insertelement <16 x i8*> %31, i8* %33, i32 9
  %35 = extractelement <16 x i64> %induction, i32 10
  %36 = getelementptr inbounds i8, i8* %a, i64 %35
  %37 = insertelement <16 x i8*> %34, i8* %36, i32 10
  %38 = extractelement <16 x i64> %induction, i32 11
  %39 = getelementptr inbounds i8, i8* %a, i64 %38
  %40 = insertelement <16 x i8*> %37, i8* %39, i32 11
  %41 = extractelement <16 x i64> %induction, i32 12
  %42 = getelementptr inbounds i8, i8* %a, i64 %41
  %43 = insertelement <16 x i8*> %40, i8* %42, i32 12
  %44 = extractelement <16 x i64> %induction, i32 13
  %45 = getelementptr inbounds i8, i8* %a, i64 %44
  %46 = insertelement <16 x i8*> %43, i8* %45, i32 13
  %47 = extractelement <16 x i64> %induction, i32 14
  %48 = getelementptr inbounds i8, i8* %a, i64 %47
  %49 = insertelement <16 x i8*> %46, i8* %48, i32 14
  %50 = extractelement <16 x i64> %induction, i32 15
  %51 = getelementptr inbounds i8, i8* %a, i64 %50
  %52 = insertelement <16 x i8*> %49, i8* %51, i32 15
  %53 = getelementptr i8, i8* %6, i32 0
  %54 = bitcast i8* %53 to <16 x i8>*
  %wide.load = load <16 x i8>, <16 x i8>* %54, align 1
  %55 = zext <16 x i8> %wide.load to <16 x i32>
  %56 = getelementptr inbounds i8, i8* %b, i64 %5
  %57 = insertelement <16 x i8*> undef, i8* %56, i32 0
  %58 = getelementptr inbounds i8, i8* %b, i64 %8
  %59 = insertelement <16 x i8*> %57, i8* %58, i32 1
  %60 = getelementptr inbounds i8, i8* %b, i64 %11
  %61 = insertelement <16 x i8*> %59, i8* %60, i32 2
  %62 = getelementptr inbounds i8, i8* %b, i64 %14
  %63 = insertelement <16 x i8*> %61, i8* %62, i32 3
  %64 = getelementptr inbounds i8, i8* %b, i64 %17
  %65 = insertelement <16 x i8*> %63, i8* %64, i32 4
  %66 = getelementptr inbounds i8, i8* %b, i64 %20
  %67 = insertelement <16 x i8*> %65, i8* %66, i32 5
  %68 = getelementptr inbounds i8, i8* %b, i64 %23
  %69 = insertelement <16 x i8*> %67, i8* %68, i32 6
  %70 = getelementptr inbounds i8, i8* %b, i64 %26
  %71 = insertelement <16 x i8*> %69, i8* %70, i32 7
  %72 = getelementptr inbounds i8, i8* %b, i64 %29
  %73 = insertelement <16 x i8*> %71, i8* %72, i32 8
  %74 = getelementptr inbounds i8, i8* %b, i64 %32
  %75 = insertelement <16 x i8*> %73, i8* %74, i32 9
  %76 = getelementptr inbounds i8, i8* %b, i64 %35
  %77 = insertelement <16 x i8*> %75, i8* %76, i32 10
  %78 = getelementptr inbounds i8, i8* %b, i64 %38
  %79 = insertelement <16 x i8*> %77, i8* %78, i32 11
  %80 = getelementptr inbounds i8, i8* %b, i64 %41
  %81 = insertelement <16 x i8*> %79, i8* %80, i32 12
  %82 = getelementptr inbounds i8, i8* %b, i64 %44
  %83 = insertelement <16 x i8*> %81, i8* %82, i32 13
  %84 = getelementptr inbounds i8, i8* %b, i64 %47
  %85 = insertelement <16 x i8*> %83, i8* %84, i32 14
  %86 = getelementptr inbounds i8, i8* %b, i64 %50
  %87 = insertelement <16 x i8*> %85, i8* %86, i32 15
  %88 = getelementptr i8, i8* %56, i32 0
  %89 = bitcast i8* %88 to <16 x i8>*
  %wide.load62 = load <16 x i8>, <16 x i8>* %89, align 1
  %90 = zext <16 x i8> %wide.load62 to <16 x i32>
  %91 = add nuw nsw <16 x i32> %90, %55
  %92 = icmp sgt <16 x i32> %91, <i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255, i32 255>
  %93 = trunc <16 x i32> %91 to <16 x i8>
  %94 = extractelement <16 x i1> %92, i32 0
  %95 = select <16 x i1> %92, <16 x i8> <i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1>, <16 x i8> %93
  %96 = bitcast i8* %88 to <16 x i8>*
  store <16 x i8> %95, <16 x i8>* %96, align 1
  %97 = add nuw nsw <16 x i64> %induction, <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
  %98 = trunc <16 x i64> %97 to <16 x i32>
  %99 = icmp eq <16 x i32> %98, %broadcast.splat64
  %index.next = add i64 %index, 16
  %100 = icmp eq i64 %index.next, %end.idx.rnd.down
  br i1 %100, label %middle.block, label %vector.body

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
  %arrayidx = getelementptr inbounds i8, i8* %a, i64 %indvars.iv
  %101 = load i8, i8* %arrayidx, align 1
  %conv = zext i8 %101 to i32
  %arrayidx2 = getelementptr inbounds i8, i8* %b, i64 %indvars.iv
  %102 = load i8, i8* %arrayidx2, align 1
  %conv3 = zext i8 %102 to i32
  %add = add nuw nsw i32 %conv3, %conv
  %cmp24 = icmp sgt i32 %add, 255
  %103 = trunc i32 %add to i8
  %conv39 = select i1 %cmp24, i8 -1, i8 %103
  store i8 %conv39, i8* %arrayidx2, align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %N
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %middle.block, %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
