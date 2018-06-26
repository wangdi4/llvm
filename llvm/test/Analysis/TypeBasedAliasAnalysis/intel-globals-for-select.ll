; RUN: opt -tbaa -gvn -S < %s | FileCheck %s

; It is illegal for GVN to pre the load(%4) of @var_78 in if.end4 because
; @var_78 is updated by the store i64 %2, i64* (%1) in entry. This load(%4)
; has a char TBAA type, but an i64 LLVM type. As Intel TBAA extension was
; treating it as a global char load which can not alias with a i64*, it
; generated no alias result for the load and eventually it was incorrectly
; removed by GVN.
; Char TBAA type assignment for any other LLVM type variable may occur with enum
; types or pointer casting.

; This test verifies that GVN does not remove the load(%4).

; CHECK: if.end4:
; CHECK: %var_78.sink = phi i64* [ @var_56, %if.then2 ], [ @var_78, %entry ]
; CHECK: %var_65.sink = phi i64* [ @var_74, %if.then2 ], [ @var_65, %entry ]
; CHECK: %4 = load i64, i64* %var_78.sink, align 8

@var_78 = external local_unnamed_addr global i64, align 8
@var_12 = external local_unnamed_addr global i64, align 8
@var_75 = external local_unnamed_addr global i64, align 8
@var_2869 = external local_unnamed_addr global i64, align 8
@var_3 = external local_unnamed_addr global i64, align 8
@var_56 = external local_unnamed_addr global i64, align 8
@var_74 = external local_unnamed_addr global i64, align 8
@var_65 = external local_unnamed_addr global i64, align 8

; Function Attrs: norecurse nounwind uwtable
define void @_Z3foov() {
entry:
  %0 = load i64, i64* @var_78, align 8, !tbaa !1
  %tobool = icmp ne i64 %0, 0
  %1 = select i1 %tobool, i64* @var_78, i64* @var_2869
  %var_12.val = load i64, i64* @var_12, align 8
  %var_75.val = load i64, i64* @var_75, align 8
  %2 = select i1 %tobool, i64 %var_12.val, i64 %var_75.val
  store i64 %2, i64* %1, align 8, !tbaa !1
  %3 = load i64, i64* @var_3, align 8, !tbaa !1
  %tobool1 = icmp eq i64 %3, 0
  br i1 %tobool1, label %if.then2, label %if.end4

if.then2:                                         ; preds = %entry
  store i64 0, i64* @var_75, align 8, !tbaa !1
  br label %if.end4

if.end4:                                          ; preds = %entry, %if.then2
  %var_78.sink = phi i64* [ @var_56, %if.then2 ], [ @var_78, %entry ]
  %var_65.sink = phi i64* [ @var_74, %if.then2 ], [ @var_65, %entry ]
  %4 = load i64, i64* %var_78.sink, align 8, !tbaa !5
  store i64 %4, i64* %var_65.sink, align 8, !tbaa !1
  ret void
}


; The following test was failing due to an incorrect rule for Selects in TBAA.
; If a ptr was a select node and one of it's arms was a global variable,
; the ptr was considered as a direct reference. Though the load(%4) is modified
; by the store i8 100, i8* %2 but TBAA generated no alias result for the select
; and the load(%4) is removed by GVN eventually.

; This test verifies that GVN does not remove the load(%4).

; CHECK: %2 = select i1 %tobool, i8* %1, i8* @g
; CHECK: %3 = load i32, i32* %t, align 4
; CHECK: store i8 %conv, i8* @g, align 1
; CHECK-NEXT: store i8 100, i8* %2, align 1
; CHECK-NEXT: %4 = load i32, i32* %t, align 4
; CHECK-NEXT: %conv1 = trunc i32 %4 to i8
; CHECK-NEXT: store i8 %conv1, i8* @h, align 1
; CHECK-NEXT: ret void

@g = dso_local global i8 1, align 1
@g2 = dso_local global i8 2, align 1
@h = dso_local global i8 3, align 1
@flag = dso_local global i32 0, align 4
@.str = private unnamed_addr constant [10 x i8] c"%d %d %d\0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local void @_Z3fooPiS_(i32* %l, i32* %t) #0 {
entry:
  %0 = load i32, i32* @flag, align 4, !tbaa !6
  %tobool = icmp eq i32 %0, 0
  %1 = bitcast i32* %l to i8*
  %2 = select i1 %tobool, i8* %1, i8* @g
  %3 = load i32, i32* %t, align 4, !tbaa !6
  %conv = trunc i32 %3 to i8
  store i8 %conv, i8* @g, align 1, !tbaa !5
  store i8 100, i8* %2, align 1, !tbaa !5
  %4 = load i32, i32* %t, align 4, !tbaa !6
  %conv1 = trunc i32 %4 to i8
  store i8 %conv1, i8* @h, align 1, !tbaa !5
  ret void
}


!0 = !{!"clang version 5.0.0 (trunk 21390)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"long long", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C++ TBAA"}
!5 = !{!3, !3, i64 0}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !3, i64 0}
