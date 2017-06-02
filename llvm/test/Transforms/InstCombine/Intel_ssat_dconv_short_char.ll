; RUN: opt < %s -instcombine -S | FileCheck %s -check-prefix=PACKSSWB
; XFAIL: *

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; the source code:
;#define T1 signed short
;#define T2 signed char
;#define MIN(x,y) ((x)>(y) ? (y) : (x))
;#define MAX(x,y) ((x)<(y) ? (y) : (x))
;#define MAX_SI8 127
;#define MIN_SI8 -128
;#define SAT2SI8(x) (MAX(MIN(x,MAX_SI8), MIN_SI8))
;
;void kernel(T1 *a, T2 *b, int N){
;  int i;
;#pragma clang loop vectorize (enable)
;#pragma clang loop vectorize_width (16)
;  for (i=0;i<N;i++){
;    b[i] = SAT2SI8(a[i]);
;  }
;}

; PACKSSWB-LABEL: kernel
; PACKSSWB: ssat.dcnv.v16i8
define void @kernel(i16* nocapture readonly %a, i8* nocapture %b, i32 %N) #0 {
entry:
  %a39 = bitcast i16* %a to i8*
  %cmp37 = icmp sgt i32 %N, 0
  br i1 %cmp37, label %for.body.preheader, label %for.end

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
  %scevgep40 = getelementptr i16, i16* %a, i64 %4
  %scevgep4041 = bitcast i16* %scevgep40 to i8*
  br i1 %cmp.zero, label %middle.block, label %vector.memcheck

vector.memcheck:                                  ; preds = %overflow.checked
  %bound0 = icmp ule i8* %b, %scevgep4041
  %bound1 = icmp ule i8* %a39, %scevgep
  %found.conflict = and i1 %bound0, %bound1
  %memcheck.conflict = and i1 %found.conflict, true
  br i1 %memcheck.conflict, label %middle.block, label %vector.ph

vector.ph:                                        ; preds = %vector.memcheck
  %broadcast.splatinsert42 = insertelement <16 x i32> undef, i32 %N, i32 0
  %broadcast.splat43 = shufflevector <16 x i32> %broadcast.splatinsert42, <16 x i32> undef, <16 x i32> zeroinitializer
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %vector.ph
  %index = phi i64 [ 0, %vector.ph ], [ %index.next, %vector.body ]
  %broadcast.splatinsert = insertelement <16 x i64> undef, i64 %index, i32 0
  %broadcast.splat = shufflevector <16 x i64> %broadcast.splatinsert, <16 x i64> undef, <16 x i32> zeroinitializer
  %induction = add <16 x i64> %broadcast.splat, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %5 = extractelement <16 x i64> %induction, i32 0
  %6 = getelementptr inbounds i16, i16* %a, i64 %5
  %7 = insertelement <16 x i16*> undef, i16* %6, i32 0
  %8 = extractelement <16 x i64> %induction, i32 1
  %9 = getelementptr inbounds i16, i16* %a, i64 %8
  %10 = insertelement <16 x i16*> %7, i16* %9, i32 1
  %11 = extractelement <16 x i64> %induction, i32 2
  %12 = getelementptr inbounds i16, i16* %a, i64 %11
  %13 = insertelement <16 x i16*> %10, i16* %12, i32 2
  %14 = extractelement <16 x i64> %induction, i32 3
  %15 = getelementptr inbounds i16, i16* %a, i64 %14
  %16 = insertelement <16 x i16*> %13, i16* %15, i32 3
  %17 = extractelement <16 x i64> %induction, i32 4
  %18 = getelementptr inbounds i16, i16* %a, i64 %17
  %19 = insertelement <16 x i16*> %16, i16* %18, i32 4
  %20 = extractelement <16 x i64> %induction, i32 5
  %21 = getelementptr inbounds i16, i16* %a, i64 %20
  %22 = insertelement <16 x i16*> %19, i16* %21, i32 5
  %23 = extractelement <16 x i64> %induction, i32 6
  %24 = getelementptr inbounds i16, i16* %a, i64 %23
  %25 = insertelement <16 x i16*> %22, i16* %24, i32 6
  %26 = extractelement <16 x i64> %induction, i32 7
  %27 = getelementptr inbounds i16, i16* %a, i64 %26
  %28 = insertelement <16 x i16*> %25, i16* %27, i32 7
  %29 = extractelement <16 x i64> %induction, i32 8
  %30 = getelementptr inbounds i16, i16* %a, i64 %29
  %31 = insertelement <16 x i16*> %28, i16* %30, i32 8
  %32 = extractelement <16 x i64> %induction, i32 9
  %33 = getelementptr inbounds i16, i16* %a, i64 %32
  %34 = insertelement <16 x i16*> %31, i16* %33, i32 9
  %35 = extractelement <16 x i64> %induction, i32 10
  %36 = getelementptr inbounds i16, i16* %a, i64 %35
  %37 = insertelement <16 x i16*> %34, i16* %36, i32 10
  %38 = extractelement <16 x i64> %induction, i32 11
  %39 = getelementptr inbounds i16, i16* %a, i64 %38
  %40 = insertelement <16 x i16*> %37, i16* %39, i32 11
  %41 = extractelement <16 x i64> %induction, i32 12
  %42 = getelementptr inbounds i16, i16* %a, i64 %41
  %43 = insertelement <16 x i16*> %40, i16* %42, i32 12
  %44 = extractelement <16 x i64> %induction, i32 13
  %45 = getelementptr inbounds i16, i16* %a, i64 %44
  %46 = insertelement <16 x i16*> %43, i16* %45, i32 13
  %47 = extractelement <16 x i64> %induction, i32 14
  %48 = getelementptr inbounds i16, i16* %a, i64 %47
  %49 = insertelement <16 x i16*> %46, i16* %48, i32 14
  %50 = extractelement <16 x i64> %induction, i32 15
  %51 = getelementptr inbounds i16, i16* %a, i64 %50
  %52 = insertelement <16 x i16*> %49, i16* %51, i32 15
  %53 = getelementptr i16, i16* %6, i32 0
  %54 = bitcast i16* %53 to <16 x i16>*
  %wide.load = load <16 x i16>, <16 x i16>* %54, align 2
  %55 = icmp sgt <16 x i16> %wide.load, <i16 127, i16 127, i16 127, i16 127, i16 127, i16 127, i16 127, i16 127, i16 127, i16 127, i16 127, i16 127, i16 127, i16 127, i16 127, i16 127>
  %56 = icmp slt <16 x i16> %wide.load, <i16 -128, i16 -128, i16 -128, i16 -128, i16 -128, i16 -128, i16 -128, i16 -128, i16 -128, i16 -128, i16 -128, i16 -128, i16 -128, i16 -128, i16 -128, i16 -128>
  %57 = xor <16 x i1> %55, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %58 = and <16 x i1> %56, %57
  %59 = trunc <16 x i16> %wide.load to <16 x i8>
  %60 = extractelement <16 x i1> %55, i32 0
  %61 = select <16 x i1> %55, <16 x i8> <i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127, i8 127>, <16 x i8> %59
  %62 = extractelement <16 x i1> %58, i32 0
  %63 = select <16 x i1> %58, <16 x i8> <i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128>, <16 x i8> %61
  %64 = getelementptr inbounds i8, i8* %b, i64 %5
  %65 = insertelement <16 x i8*> undef, i8* %64, i32 0
  %66 = getelementptr inbounds i8, i8* %b, i64 %8
  %67 = insertelement <16 x i8*> %65, i8* %66, i32 1
  %68 = getelementptr inbounds i8, i8* %b, i64 %11
  %69 = insertelement <16 x i8*> %67, i8* %68, i32 2
  %70 = getelementptr inbounds i8, i8* %b, i64 %14
  %71 = insertelement <16 x i8*> %69, i8* %70, i32 3
  %72 = getelementptr inbounds i8, i8* %b, i64 %17
  %73 = insertelement <16 x i8*> %71, i8* %72, i32 4
  %74 = getelementptr inbounds i8, i8* %b, i64 %20
  %75 = insertelement <16 x i8*> %73, i8* %74, i32 5
  %76 = getelementptr inbounds i8, i8* %b, i64 %23
  %77 = insertelement <16 x i8*> %75, i8* %76, i32 6
  %78 = getelementptr inbounds i8, i8* %b, i64 %26
  %79 = insertelement <16 x i8*> %77, i8* %78, i32 7
  %80 = getelementptr inbounds i8, i8* %b, i64 %29
  %81 = insertelement <16 x i8*> %79, i8* %80, i32 8
  %82 = getelementptr inbounds i8, i8* %b, i64 %32
  %83 = insertelement <16 x i8*> %81, i8* %82, i32 9
  %84 = getelementptr inbounds i8, i8* %b, i64 %35
  %85 = insertelement <16 x i8*> %83, i8* %84, i32 10
  %86 = getelementptr inbounds i8, i8* %b, i64 %38
  %87 = insertelement <16 x i8*> %85, i8* %86, i32 11
  %88 = getelementptr inbounds i8, i8* %b, i64 %41
  %89 = insertelement <16 x i8*> %87, i8* %88, i32 12
  %90 = getelementptr inbounds i8, i8* %b, i64 %44
  %91 = insertelement <16 x i8*> %89, i8* %90, i32 13
  %92 = getelementptr inbounds i8, i8* %b, i64 %47
  %93 = insertelement <16 x i8*> %91, i8* %92, i32 14
  %94 = getelementptr inbounds i8, i8* %b, i64 %50
  %95 = insertelement <16 x i8*> %93, i8* %94, i32 15
  %96 = getelementptr i8, i8* %64, i32 0
  %97 = bitcast i8* %96 to <16 x i8>*
  store <16 x i8> %63, <16 x i8>* %97, align 1
  %98 = add nuw nsw <16 x i64> %induction, <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
  %99 = trunc <16 x i64> %98 to <16 x i32>
  %100 = icmp eq <16 x i32> %99, %broadcast.splat43
  %index.next = add i64 %index, 16
  %101 = icmp eq i64 %index.next, %end.idx.rnd.down
  br i1 %101, label %middle.block, label %vector.body

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
  %102 = load i16, i16* %arrayidx, align 2
  %cmp1 = icmp sgt i16 %102, 127
  %cmp636 = icmp slt i16 %102, -128
  %not.cmp1 = xor i1 %cmp1, true
  %cmp6 = and i1 %cmp636, %not.cmp1
  %conv12 = trunc i16 %102 to i8
  %.conv12 = select i1 %cmp1, i8 127, i8 %conv12
  %cond23 = select i1 %cmp6, i8 -128, i8 %.conv12
  %arrayidx26 = getelementptr inbounds i8, i8* %b, i64 %indvars.iv
  store i8 %cond23, i8* %arrayidx26, align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %N
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %middle.block, %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

