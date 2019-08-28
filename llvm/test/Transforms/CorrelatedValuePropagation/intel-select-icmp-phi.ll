; RUN: opt < %s --correlated-propagation -instcombine -S | FileCheck %s

define dso_local i32 @test_select_icmp_phi(i64 %a) local_unnamed_addr {
; CHECK-LABEL: @test_select_icmp_phi(
; CHECK:       %0 = trunc i64 %a to i32
; CHECK-NEXT:  %1 = icmp ult i32 %0, 2
; CHECK-NEXT:  br i1 %1, label %bb0, label %bb1

entry:
  br label %bb0

  bb0:                                          ; preds = %entry, %bb0
    %b = phi i64 [ 0, %entry ], [ %b, %bb0 ]
    %0 = call i64 @llvm.cttz.i64(i64 %b, i1 true)
    %1 = icmp eq i64 %b, 0
    %2 = select i1 %1, i64 %a, i64 %0
    %3 = trunc i64 %2 to i32
    %4 = icmp ult i32 %3, 2
    br i1 %4, label %bb0, label %bb1

  bb1:                  ; preds = %bb0
    ret i32 %3
}

; Function Attrs: nounwind readnone speculatable
declare i64 @llvm.cttz.i64(i64, i1 immarg)
