; This test verifies that the field reordering transformation is applied
; to struct.test which has a large number of fields, and a new struct is
; created with different layout.
;
; THe layout of struct.S is modeled under cpu2017/520-omnetpp:cMessage type:
; - field0 is a struct type
; - total of 20 fields;
; - 13 integer-type fields;
; -  5 struct-type fields;
; -  3 pointer-type fields;
;

;  RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -S -passes=dtrans-reorderfieldsop -dtrans-reorderfieldop-enable-applicable-test=0 -dtrans-reorderfieldop-enable-legal-test=0 | FileCheck %s

; CHECK: %__DFR_struct.S = type { %struct.S0, i32, i64, i16, i16, i16, i64, i64, i64, ptr, ptr, ptr, i32, i32, %struct.S0, %struct.S0, %struct.S0, %struct.S0, i32, i32 }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S = type { %struct.S0, i32, i64, i32, i16, i32, i16, i32, i32, i64, i64, i64, i16, %struct.S0, %struct.S0, %struct.S0, %struct.S0, ptr, ptr, ptr }
%struct.S0 = type { i32 }

@.str = private unnamed_addr constant [10 x i8] c"sum: %lu\0A\00", align 1

define dso_local i64 @work(ptr "intel_dtrans_func_index"="1" nocapture %s, i32 %UB1, i32 %UB2) local_unnamed_addr  !intel.dtrans.func.type !25 {
entry:
  %f1 = getelementptr inbounds %struct.S, ptr %s, i64 0, i32 1, !intel-tbaa !2
  store i32 %UB1, ptr %f1, align 4, !tbaa !2
  %conv = trunc i32 %UB2 to i16
  %f4 = getelementptr inbounds %struct.S, ptr %s, i64 0, i32 4, !intel-tbaa !13
  store i16 %conv, ptr %f4, align 4, !tbaa !13
  %f7 = getelementptr inbounds %struct.S, ptr %s, i64 0, i32 7, !intel-tbaa !14
  store i32 %UB2, ptr %f7, align 8, !tbaa !14
  %conv1 = sext i32 %UB1 to i64
  %f11 = getelementptr inbounds %struct.S, ptr %s, i64 0, i32 11, !intel-tbaa !15
  store i64 %conv1, ptr %f11, align 8, !tbaa !15
  %cmp59 = icmp sgt i32 %UB1, 0
  %f3 = getelementptr inbounds %struct.S, ptr %s, i64 0, i32 3
  %f3.promoted = load i32, ptr %f3, align 8, !tbaa !16
  br i1 %cmp59, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = add i32 %UB1, -1
  %1 = lshr i32 %0, 1
  %2 = add nuw i32 %1, 1
  %xtraiter = and i32 %2, 1
  %3 = icmp eq i32 %1, 0
  br i1 %3, label %for.cond.for.cond.cleanup_crit_edge.unr-lcssa, label %for.body.preheader.new

for.body.preheader.new:                           ; preds = %for.body.preheader
  %unroll_iter = sub nuw i32 %2, %xtraiter
  br label %for.body

for.cond.for.cond.cleanup_crit_edge.unr-lcssa:    ; preds = %for.body, %for.body.preheader
  %add4.lcssa.ph = phi i32 [ undef, %for.body.preheader ], [ %add4.1, %for.body ]
  %add8.lcssa.ph = phi i32 [ undef, %for.body.preheader ], [ %add8.1, %for.body ]
  %conv15.lcssa.ph = phi i32 [ undef, %for.body.preheader ], [ %conv15.1, %for.body ]
  %add20.lcssa.ph = phi i64 [ undef, %for.body.preheader ], [ %add20.1, %for.body ]
  %add2063.unr = phi i64 [ %conv1, %for.body.preheader ], [ %add20.1, %for.body ]
  %conv1562.unr = phi i32 [ %UB2, %for.body.preheader ], [ %conv15.1, %for.body ]
  %.unr = phi i32 [ %UB1, %for.body.preheader ], [ %add4.1, %for.body ]
  %add861.unr = phi i32 [ %f3.promoted, %for.body.preheader ], [ %add8.1, %for.body ]
  %i.060.unr = phi i32 [ 0, %for.body.preheader ], [ %add21.1, %for.body ]
  %lcmp.mod = icmp eq i32 %xtraiter, 0
  br i1 %lcmp.mod, label %for.cond.for.cond.cleanup_crit_edge, label %for.body.epil

for.body.epil:                                    ; preds = %for.cond.for.cond.cleanup_crit_edge.unr-lcssa
  %add.epil = add i32 %add861.unr, %i.060.unr
  %add8.epil = add i32 %add.epil, %conv1562.unr
  %add17.epil = add i32 %add8.epil, %i.060.unr
  %conv18.epil = zext i32 %add17.epil to i64
  %add20.epil = add i64 %add2063.unr, %conv18.epil
  %4 = add i32 %conv1562.unr, %i.060.unr
  %5 = trunc i64 %add2063.unr to i32
  %conv15.epil = add i32 %4, %5
  %add4.epil = add i32 %add.epil, %.unr
  br label %for.cond.for.cond.cleanup_crit_edge

for.cond.for.cond.cleanup_crit_edge:              ; preds = %for.cond.for.cond.cleanup_crit_edge.unr-lcssa, %for.body.epil
  %add4.lcssa = phi i32 [ %add4.lcssa.ph, %for.cond.for.cond.cleanup_crit_edge.unr-lcssa ], [ %add4.epil, %for.body.epil ]
  %add8.lcssa = phi i32 [ %add8.lcssa.ph, %for.cond.for.cond.cleanup_crit_edge.unr-lcssa ], [ %add8.epil, %for.body.epil ]
  %conv15.lcssa = phi i32 [ %conv15.lcssa.ph, %for.cond.for.cond.cleanup_crit_edge.unr-lcssa ], [ %conv15.epil, %for.body.epil ]
  %add20.lcssa = phi i64 [ %add20.lcssa.ph, %for.cond.for.cond.cleanup_crit_edge.unr-lcssa ], [ %add20.epil, %for.body.epil ]
  store i32 %add8.lcssa, ptr %f3, align 8, !tbaa !16
  store i32 %add4.lcssa, ptr %f1, align 4, !tbaa !2
  store i32 %conv15.lcssa, ptr %f7, align 8, !tbaa !14
  store i64 %add20.lcssa, ptr %f11, align 8, !tbaa !15
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %entry, %for.cond.for.cond.cleanup_crit_edge
  %6 = phi i64 [ %add20.lcssa, %for.cond.for.cond.cleanup_crit_edge ], [ %conv1, %entry ]
  %7 = phi i32 [ %conv15.lcssa, %for.cond.for.cond.cleanup_crit_edge ], [ %UB2, %entry ]
  %8 = phi i32 [ %add8.lcssa, %for.cond.for.cond.cleanup_crit_edge ], [ %f3.promoted, %entry ]
  %9 = phi i32 [ %add4.lcssa, %for.cond.for.cond.cleanup_crit_edge ], [ %UB1, %entry ]
  %conv26 = and i32 %UB2, 65535
  %add24 = add i32 %9, %conv26
  %add27 = add i32 %add24, %8
  %add29 = add i32 %add27, %7
  %conv30 = zext i32 %add29 to i64
  %add32 = add i64 %6, %conv30
  ret i64 %add32

for.body:                                         ; preds = %for.body, %for.body.preheader.new
  %add2063 = phi i64 [ %conv1, %for.body.preheader.new ], [ %add20.1, %for.body ]
  %conv1562 = phi i32 [ %UB2, %for.body.preheader.new ], [ %conv15.1, %for.body ]
  %10 = phi i32 [ %UB1, %for.body.preheader.new ], [ %add4.1, %for.body ]
  %add861 = phi i32 [ %f3.promoted, %for.body.preheader.new ], [ %add8.1, %for.body ]
  %i.060 = phi i32 [ 0, %for.body.preheader.new ], [ %add21.1, %for.body ]
  %niter = phi i32 [ %unroll_iter, %for.body.preheader.new ], [ %niter.nsub.1, %for.body ]
  %add = add i32 %add861, %i.060
  %add4 = add i32 %add, %10
  %add8 = add i32 %add, %conv1562
  %11 = trunc i64 %add2063 to i32
  %12 = add i32 %conv1562, %i.060
  %conv15 = add i32 %12, %11
  %add17 = add i32 %add8, %i.060
  %conv18 = zext i32 %add17 to i64
  %add20 = add i64 %add2063, %conv18
  %add21 = or i32 %i.060, 2
  %add.1 = add i32 %add8, %add21
  %add4.1 = add i32 %add.1, %add4
  %add8.1 = add i32 %add.1, %conv15
  %13 = trunc i64 %add20 to i32
  %14 = add i32 %conv15, %add21
  %conv15.1 = add i32 %14, %13
  %add17.1 = add i32 %add8.1, %add21
  %conv18.1 = zext i32 %add17.1 to i64
  %add20.1 = add i64 %add20, %conv18.1
  %add21.1 = add nuw nsw i32 %i.060, 4
  %niter.nsub.1 = add i32 %niter, -2
  %niter.ncmp.1 = icmp eq i32 %niter.nsub.1, 0
  br i1 %niter.ncmp.1, label %for.cond.for.cond.cleanup_crit_edge.unr-lcssa, label %for.body
}

define i32 @main(i32 %argc, ptr "intel_dtrans_func_index"="1" nocapture readnone %argv) !intel.dtrans.func.type !27 {
entry:
  %call = tail call noalias ptr @malloc(i64 112) #3
  %add = add nsw i32 %argc, 100
  %call1 = tail call i64 @work(ptr %call, i32 %argc, i32 %add)
  %call2 = tail call i32 (ptr, ...) @printf(ptr getelementptr inbounds ([10 x i8], ptr @.str, i64 0, i64 0), i64 %call1)
  tail call void @free(ptr %call)
  ret i32 0
}

declare !intel.dtrans.func.type !28 "intel_dtrans_func_index"="1" ptr @malloc(i64)
declare !intel.dtrans.func.type !29 i32 @printf(ptr "intel_dtrans_func_index"="1" nocapture readonly, ...)
declare !intel.dtrans.func.type !30 void @free(ptr "intel_dtrans_func_index"="1" nocapture)

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
!17 = !{%struct.S0 zeroinitializer, i32 0}  ; %struct.S0
!18 = !{i32 0, i32 0}  ; i32
!19 = !{i64 0, i32 0}  ; i64
!20 = !{i16 0, i32 0}  ; i16
!21 = !{i8 0, i32 1}  ; i8*
!22 = !{i32 0, i32 1}  ; i32*
!23 = !{i16 0, i32 1}  ; i16*
!24 = !{%struct.S zeroinitializer, i32 1}  ; %struct.S*
!25 = distinct !{!24}
!26 = !{i8 0, i32 2}  ; i8**
!27 = distinct !{!26}
!28 = distinct !{!21}
!29 = distinct !{!21}
!30 = distinct !{!21}
!31 = !{!"S", %struct.S zeroinitializer, i32 20, !17, !18, !19, !18, !20, !18, !20, !18, !18, !19, !19, !19, !20, !17, !17, !17, !17, !21, !22, !23} ; { %struct.S0, i32, i64, i32, i16, i32, i16, i32, i32, i64, i64, i64, i16, %struct.S0, %struct.S0, %struct.S0, %struct.S0, i8*, i32*, i16* }
!32 = !{!"S", %struct.S0 zeroinitializer, i32 1, !18} ; { i32 }

!intel.dtrans.types = !{!31, !32}

;
; *** Source Code ***
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
;  return 0;
;}
;
