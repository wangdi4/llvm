; This test verifies that reorder-field transformation supports limited form of byte-flatten GEP on a structure that
; contains a complete copy of the struct being reordered.
;

;  RUN: opt  -whole-program-assume < %s -S -dtrans-reorderfields -dtrans-reorderfield-enable-applicable-test=0 -dtrans-reorderfield-enable-legal-test=0 | FileCheck %s
;  RUN: opt  -whole-program-assume < %s -S -passes=dtrans-reorderfields -dtrans-reorderfield-enable-applicable-test=0 -dtrans-reorderfield-enable-legal-test=0 | FileCheck %s

; check __DFR_struct.S means DTrans Field-Reorder pass triggered on this struct type
; CHECK: %__DFR_struct.S = type { %struct.S0, i32, i64, i64, i16, i16, i32, i64, i64, i8*, i32*, i16*, i32, i32, %struct.S0, %struct.S0, %struct.S0, %struct.S0, i32, i16 }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S = type { %struct.S0, i32, i64, i32, i16, i32, i16, i32, i32, i64, i64, i64, i16, %struct.S0, %struct.S0, %struct.S0, %struct.S0, i8*, i32*, i16* }
%struct.S0 = type { i32 }
%struct.T = type { %struct.S, i32 }

@.str = private unnamed_addr constant [10 x i8] c"t->i: %d\0A\00", align 1
@.str.1 = private unnamed_addr constant [10 x i8] c"sum: %lu\0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local i64 @workST(%struct.S* nocapture %s, %struct.T* nocapture %t, i32 %UB1, i32 %UB2) local_unnamed_addr #0 {
entry:
  %f1 = getelementptr inbounds %struct.S, %struct.S* %s, i64 0, i32 1, !intel-tbaa !2
  store i32 %UB1, i32* %f1, align 4, !tbaa !2
  %conv = trunc i32 %UB2 to i16
  %f4 = getelementptr inbounds %struct.S, %struct.S* %s, i64 0, i32 4, !intel-tbaa !13
  store i16 %conv, i16* %f4, align 4, !tbaa !13
  %f7 = getelementptr inbounds %struct.S, %struct.S* %s, i64 0, i32 7, !intel-tbaa !14
  store i32 %UB2, i32* %f7, align 8, !tbaa !14
  %conv1 = sext i32 %UB1 to i64
  %f11 = getelementptr inbounds %struct.S, %struct.S* %s, i64 0, i32 11, !intel-tbaa !15
  store i64 %conv1, i64* %f11, align 8, !tbaa !15
  %cmp63 = icmp sgt i32 %UB1, 0
  br i1 %cmp63, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %f3 = getelementptr inbounds %struct.S, %struct.S* %s, i64 0, i32 3, !intel-tbaa !16
  %f3.promoted = load i32, i32* %f3, align 8, !tbaa !16
  %0 = add i32 %UB1, -1
  %1 = lshr i32 %0, 1
  %2 = add nuw i32 %1, 1
  %xtraiter = and i32 %2, 1
  %3 = icmp eq i32 %1, 0
  br i1 %3, label %for.cond.for.cond.cleanup_crit_edge.unr-lcssa, label %for.body.lr.ph.new

for.body.lr.ph.new:                               ; preds = %for.body.lr.ph
  %unroll_iter = sub nuw i32 %2, %xtraiter
  br label %for.body

for.cond.for.cond.cleanup_crit_edge.unr-lcssa:    ; preds = %for.body, %for.body.lr.ph
  %add4.lcssa.ph = phi i32 [ undef, %for.body.lr.ph ], [ %add4.1, %for.body ]
  %add8.lcssa.ph = phi i32 [ undef, %for.body.lr.ph ], [ %add8.1, %for.body ]
  %conv15.lcssa.ph = phi i32 [ undef, %for.body.lr.ph ], [ %conv15.1, %for.body ]
  %add20.lcssa.ph = phi i64 [ undef, %for.body.lr.ph ], [ %add20.1, %for.body ]
  %add2067.unr = phi i64 [ %conv1, %for.body.lr.ph ], [ %add20.1, %for.body ]
  %conv1566.unr = phi i32 [ %UB2, %for.body.lr.ph ], [ %conv15.1, %for.body ]
  %.unr = phi i32 [ %UB1, %for.body.lr.ph ], [ %add4.1, %for.body ]
  %add865.unr = phi i32 [ %f3.promoted, %for.body.lr.ph ], [ %add8.1, %for.body ]
  %i.064.unr = phi i32 [ 0, %for.body.lr.ph ], [ %add21.1, %for.body ]
  %lcmp.mod = icmp eq i32 %xtraiter, 0
  br i1 %lcmp.mod, label %for.cond.for.cond.cleanup_crit_edge, label %for.body.epil

for.body.epil:                                    ; preds = %for.cond.for.cond.cleanup_crit_edge.unr-lcssa
  %add.epil = add i32 %add865.unr, %i.064.unr
  %add8.epil = add i32 %add.epil, %conv1566.unr
  %add17.epil = add i32 %add8.epil, %i.064.unr
  %conv18.epil = zext i32 %add17.epil to i64
  %add20.epil = add i64 %add2067.unr, %conv18.epil
  %4 = add i32 %conv1566.unr, %i.064.unr
  %5 = trunc i64 %add2067.unr to i32
  %conv15.epil = add i32 %4, %5
  %add4.epil = add i32 %add.epil, %.unr
  br label %for.cond.for.cond.cleanup_crit_edge

for.cond.for.cond.cleanup_crit_edge:              ; preds = %for.cond.for.cond.cleanup_crit_edge.unr-lcssa, %for.body.epil
  %add4.lcssa = phi i32 [ %add4.lcssa.ph, %for.cond.for.cond.cleanup_crit_edge.unr-lcssa ], [ %add4.epil, %for.body.epil ]
  %add8.lcssa = phi i32 [ %add8.lcssa.ph, %for.cond.for.cond.cleanup_crit_edge.unr-lcssa ], [ %add8.epil, %for.body.epil ]
  %conv15.lcssa = phi i32 [ %conv15.lcssa.ph, %for.cond.for.cond.cleanup_crit_edge.unr-lcssa ], [ %conv15.epil, %for.body.epil ]
  %add20.lcssa = phi i64 [ %add20.lcssa.ph, %for.cond.for.cond.cleanup_crit_edge.unr-lcssa ], [ %add20.epil, %for.body.epil ]
  store i32 %add8.lcssa, i32* %f3, align 8, !tbaa !16
  store i32 %add4.lcssa, i32* %f1, align 4, !tbaa !2
  store i32 %conv15.lcssa, i32* %f7, align 8, !tbaa !14
  store i64 %add20.lcssa, i64* %f11, align 8, !tbaa !15
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.for.cond.cleanup_crit_edge, %entry
  %div = sdiv i32 %UB1, 2

  ;[ORIG]
  ;%i22 = getelementptr inbounds %struct.T, %struct.T* %t, i64 0, i32 1, !intel-tbaa !17
  ;
  ;[NEW CODE SEQUENCE]
  ;[Do a byte-flatten GEP inside a DFR struct]
  ;
  %row0 = bitcast %struct.S* %s to i8*
  %addr0= getelementptr i8, i8* %row0, i64 8
  %val0 = bitcast i8* %addr0 to i32*
  store i32 %div, i32* %val0, align 8, !tbaa !17

;CHECK-dag:  %addr0= getelementptr i8, i8* %row0, i64 16

  ;[NEW CODE SEQUENCE]
  ;[Do a byte-flatten GEP inside a Inclusive-StructType DFR struct, in DFR range]
  ;
  %row1 = bitcast %struct.T* %t to i8*
  %addr1= getelementptr i8, i8* %row1, i64 8
  %val1 = bitcast i8* %addr1 to i32*
  store i32 %div, i32* %val1, align 8, !tbaa !17

;CHECK-dag:  %addr1= getelementptr i8, i8* %row0, i64 16

  ;[NEW CODE SEQUENCE]
  ;[Do a byte-flatten GEP inside a Inclusive-StructType DFR struct, out of DFR range]
  ;
  %row2 = bitcast %struct.T* %t to i8*
  %addr2= getelementptr i8, i8* %row2, i64 112
  %val2 = bitcast i8* %addr1 to i32*
  store i32 %div, i32* %val2, align 8, !tbaa !17

;CHECK-dag:  %addr2= getelementptr i8, i8* %row0, i64 104

  %call = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str, i64 0, i64 0), i32 %div)
  %6 = load i32, i32* %f1, align 4, !tbaa !2
  %f325 = getelementptr inbounds %struct.S, %struct.S* %s, i64 0, i32 3, !intel-tbaa !16
  %7 = load i32, i32* %f325, align 8, !tbaa !16
  %add26 = add i32 %7, %6
  %8 = load i16, i16* %f4, align 4, !tbaa !13
  %conv28 = zext i16 %8 to i32
  %add29 = add i32 %add26, %conv28
  %9 = load i32, i32* %f7, align 8, !tbaa !14
  %add31 = add i32 %add29, %9
  %conv32 = zext i32 %add31 to i64
  %10 = load i64, i64* %f11, align 8, !tbaa !15
  %add34 = add i64 %10, %conv32
  ret i64 %add34

for.body:                                         ; preds = %for.body, %for.body.lr.ph.new
  %add2067 = phi i64 [ %conv1, %for.body.lr.ph.new ], [ %add20.1, %for.body ]
  %conv1566 = phi i32 [ %UB2, %for.body.lr.ph.new ], [ %conv15.1, %for.body ]
  %11 = phi i32 [ %UB1, %for.body.lr.ph.new ], [ %add4.1, %for.body ]
  %add865 = phi i32 [ %f3.promoted, %for.body.lr.ph.new ], [ %add8.1, %for.body ]
  %i.064 = phi i32 [ 0, %for.body.lr.ph.new ], [ %add21.1, %for.body ]
  %niter = phi i32 [ %unroll_iter, %for.body.lr.ph.new ], [ %niter.nsub.1, %for.body ]
  %add = add i32 %add865, %i.064
  %add4 = add i32 %add, %11
  %add8 = add i32 %add, %conv1566
  %12 = trunc i64 %add2067 to i32
  %13 = add i32 %conv1566, %i.064
  %conv15 = add i32 %13, %12
  %add17 = add i32 %add8, %i.064
  %conv18 = zext i32 %add17 to i64
  %add20 = add i64 %add2067, %conv18
  %add21 = or i32 %i.064, 2
  %add.1 = add i32 %add8, %add21
  %add4.1 = add i32 %add.1, %add4
  %add8.1 = add i32 %add.1, %conv15
  %14 = trunc i64 %add20 to i32
  %15 = add i32 %conv15, %add21
  %conv15.1 = add i32 %15, %14
  %add17.1 = add i32 %add8.1, %add21
  %conv18.1 = zext i32 %add17.1 to i64
  %add20.1 = add i64 %add20, %conv18.1
  %add21.1 = add nuw nsw i32 %i.064, 4
  %niter.nsub.1 = add i32 %niter, -2
  %niter.ncmp.1 = icmp eq i32 %niter.nsub.1, 0
  br i1 %niter.ncmp.1, label %for.cond.for.cond.cleanup_crit_edge.unr-lcssa, label %for.body
}

; Function Attrs: nounwind
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define dso_local i32 @main(i32 %argc, i8** nocapture readnone %argv) local_unnamed_addr #2 {
entry:
  %call = tail call noalias i8* @malloc(i64 112) #3
  %0 = bitcast i8* %call to %struct.S*
  %call1 = tail call noalias i8* @malloc(i64 120) #3
  %1 = bitcast i8* %call1 to %struct.T*
  %add = add nsw i32 %argc, 100
  %call2 = tail call i64 @workST(%struct.S* %0, %struct.T* %1, i32 %argc, i32 %add)
  %call3 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str.1, i64 0, i64 0), i64 %call2)
  tail call void @free(i8* %call) #3
  tail call void @free(i8* %call1) #3
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @malloc(i64) local_unnamed_addr #1

; Function Attrs: nounwind
declare dso_local void @free(i8* nocapture) local_unnamed_addr #1

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !5, i64 4}
!3 = !{!"struct@S", !4, i64 0, !5, i64 4, !8, i64 8, !5, i64 16, !9, i64 20, !5, i64 24, !9, i64 28, !5, i64 32, !5, i64 36, !8, i64 40, !8, i64 48, !8, i64 56, !9, i64 64, !4, i64 68, !4, i64 72, !4, i64 76, !4, i64 80, !10, i64 88, !11, i64 96, !12, i64 104}
!4 = !{!"struct@S0", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!"long", !6, i64 0}
!9 = !{!"short", !6, i64 0}
!10 = !{!"pointer@_ZTSPc", !6, i64 0}
!11 = !{!"pointer@_ZTSPi", !6, i64 0}
!12 = !{!"pointer@_ZTSPs", !6, i64 0}
!13 = !{!3, !9, i64 20}
!14 = !{!3, !5, i64 32}
!15 = !{!3, !8, i64 56}
!16 = !{!3, !5, i64 16}
!17 = !{!18, !5, i64 112}
!18 = !{!"struct@T", !3, i64 0, !5, i64 112}

;
; *** Source Code ***
;
; cmdline: icx test.c -O0 -S -emit-llvm
;
;#include <stdint.h>
;#include <stdio.h>
;#include <stdlib.h>
;
;struct S0 {
;  int d;
;};
;
;// note: a large struct with 20 fields of integer types, struct types, and pointer types.
;struct S {
;  struct S0 f0; // 1 struct type: model inheritance
;  uint32_t f1;
;  uint64_t f2;
;  uint32_t f3;
;  uint16_t f4;
;  uint32_t f5;
;  uint16_t f6;
;  uint32_t f7;
;  uint32_t f8;
;  uint64_t f9;
;  uint64_t f10;
;  uint64_t f11;
;  uint16_t f12;
;  struct S0 f13; // 4 struct types
;  struct S0 f14;
;  struct S0 f15;
;  struct S0 f16;
;  char *f17; // 3 pointer types
;  int *f18;
;  short *f19;
;};
;
;// type T contains a complete data layout of type S, which is being reordered
;struct T {
;  struct S base;
;  int i;
;}
;
;uint64_t __attribute__((noinline)) work(struct S *s, int UB1, int UB2) {
;  // ONE-time field assign
;  s->f1 = UB1;
;  s->f4 = UB2;
;  s->f7 = UB2;
;  s->f11 = UB1;
;
;  // single-loop assignment with an unknown trip count
;  for (int i = 0; i < UB1; i += 2) {
;    s->f1 += s->f3 + i;
;    s->f3 += s->f7 + i;
;    s->f7 += s->f11 + i;
;    s->f11 += s->f3 + i;
;  }
;
;  // do a sum, so all fields are used
;  uint64_t sum = s->f1 + s->f3 + s->f4 + s->f7 + s->f11;
;
;  return sum;
;}
;
;int main(int argc, char *argv[]) {
;  struct S *s = malloc(sizeof(struct S));
;  uint64_t sum = work(s, argc, argc + 100);
;  printf("sum: %lu\n", sum);
;  free(s);
;  T t;
;  t.i = argc;
;  printf("T.i: %d\n",t,i);
;  return 0;
;}
;
;
