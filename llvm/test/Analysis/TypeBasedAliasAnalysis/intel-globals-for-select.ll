; RUN: opt -tbaa -gvn -S < %s | FileCheck %s

; It is illegal for GVN to pre the load(%4) of @var_78 in if.end4 because
; @var_78 is updated by the store i64 %2, i64* (%1) in entry.
; GVN removes this load because TBAA does not recognize direct
; references due to lack of proper 'select' node analysis.
; This test verifies that GVN does not remove this load.

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

!0 = !{!"clang version 5.0.0 (trunk 21390)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"long long", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C++ TBAA"}
!5 = !{!3, !3, i64 0}
