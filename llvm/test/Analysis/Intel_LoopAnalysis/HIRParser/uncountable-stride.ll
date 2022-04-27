; RUN: opt -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-framework -hir-framework-debug=parser -hir-details <%s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-details -hir-framework-debug=parser -disable-output <%s  2>&1 | FileCheck %s

; Test checks that HIR parser does not assert when it cannot calculate a stride for header phi index CE.

; CHECK: BEGIN REGION

; EntryBB: %1
; ExitBB: %7
; Member BBlocks: %1, %4, %11, %18, %20, %5, %7
; LiveIns: %3(getelementptr inbounds ([60 x i8], [60 x i8]* @"_unnamed_main$$_$RES", i64 0, i64 0)), @strlit.4(@strlit.4), @strlit.5(@strlit.5), %2(0)
; LiveOuts: %8(sym:5)
; <50>
; <50>               + DO i1 = 0, 3, 1   <DO_LOOP>
; <2>                |   %8 = &((%3)[0:0:0(i8*:0)]);
; <3>                |   if (undef true undef)
; <3>                |   {
; <51>               |
; <51>               |      + DO i2 = 0, 1, 1   <DO_LOOP>
; <12>               |      |   %15 = (@strlit.5)[0:0:2([2 x i8]*:0)][0:i2:1([2 x i8]:2)];
; <14>               |      |   (%3)[0:i2:1(i8*:0)] = %15;
; <51>               |      + END LOOP
; <51>               |
; <52>               |
; <52>               |      + DO i2 = 0, 2, 1   <DO_LOOP>
; <28>               |      |   %24 = (@strlit.4)[0:0:3([3 x i8]*:0)][0:i2:1([3 x i8]:3)];
; <30>               |      |   (%3)[0:i2 + 2:1(i8*:0)] = %24;
; <52>               |      + END LOOP
; <52>               |
; <40>               |      %8 = &((%3)[0:5:1(i8*:0)]);
; <3>                |   }
; <45>               |   %3 = &((%8)[0:0:0(i8*:0)]);
; <50>               + END LOOP
; <50>
; <0>          END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@strlit.4 = internal unnamed_addr constant [3 x i8] c"nd.", !llfort.type_idx !3
@strlit.5 = internal unnamed_addr constant [2 x i8] c" E", !llfort.type_idx !0
@"_unnamed_main$$_$RES" = internal global [60 x i8] zeroinitializer, align 8, !llfort.type_idx !5

; Function Attrs: nofree norecurse nosync nounwind writeonly uwtable
define internal fastcc void @f() unnamed_addr {
  br label %1

1:                                                ; preds = %7, %0
  %2 = phi i32 [ %9, %7 ], [ 0, %0 ]
  %3 = phi i8* [ %8, %7 ], [ getelementptr inbounds ([60 x i8], [60 x i8]* @"_unnamed_main$$_$RES", i64 0, i64 0), %0 ]
  br i1 true, label %4, label %7

4:                                                ; preds = %1
  br label %11

5:                                                ; preds = %20
  %6 = getelementptr i8, i8* %3, i64 5
  br label %7

7:                                                ; preds = %5, %1
  %8 = phi i8* [ %3, %1 ], [ %6, %5 ]
  %9 = add nuw nsw i32 %2, 1
  %10 = icmp ne i32 %9, 4
  br i1 %10, label %1, label %28

11:                                               ; preds = %4, %11
  %12 = phi i64 [ 1, %11 ], [ 0, %4 ]
  %13 = phi i8* [ %16, %11 ], [ %3, %4 ]
  %14 = getelementptr inbounds [2 x i8], [2 x i8]* @strlit.5, i64 0, i64 %12
  %15 = load i8, i8* %14, align 1
  %16 = getelementptr inbounds i8, i8* %13, i64 1
  store i8 %15, i8* %13, align 1
  %17 = icmp eq i64 %12, 0
  br i1 %17, label %11, label %18

18:                                               ; preds = %11
  %19 = getelementptr i8, i8* %3, i64 2
  br label %20

20:                                               ; preds = %18, %20
  %21 = phi i64 [ %26, %20 ], [ 0, %18 ]
  %22 = phi i8* [ %25, %20 ], [ %19, %18 ]
  %23 = getelementptr inbounds [3 x i8], [3 x i8]* @strlit.4, i64 0, i64 %21
  %24 = load i8, i8* %23, align 1
  %25 = getelementptr inbounds i8, i8* %22, i64 1
  store i8 %24, i8* %22, align 1
  %26 = add nuw nsw i64 %21, 1
  %27 = icmp ne i64 %26, 3
  br i1 %27, label %20, label %5

28:                                               ; preds = %7
  %29 = phi i8* [ %8, %7 ]
  br label %30

30:                                               ; preds = %28
  ret void
}

!0 = !{i64 38}
!1 = !{i64 40}
!2 = !{i64 41}
!3 = !{i64 42}
!4 = !{i64 43}
!5 = !{i64 48}

