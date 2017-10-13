; RUN: opt -S -mtriple=csa -csa-fortran-intrinsics < %s | FileCheck %s

; ModuleID = 'intrinsic.for'
source_filename = "intrinsic.for"
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind uwtable
define i32 @test_(i32* noalias nocapture readonly %a, i32* noalias nocapture readonly %b) unnamed_addr #0 {
entry:
; CHECK: call void @llvm.csa.parallel.loop()
; CHECK: call void @llvm.csa.parallel.loop()
; CHECK: call void @llvm.csa.parallel.loop()
  tail call void bitcast (void (...)* @builtin_csa_parallel_loop_ to void ()*)() #1
  tail call void bitcast (void (...)* @builtin_csa_parallel_loop_ to void ()*)() #1
  %0 = load i32, i32* %a, align 4, !tbaa !1
  %1 = load i32, i32* %b, align 4, !tbaa !1
  %2 = add nsw i32 %1, %0
  tail call void bitcast (void (...)* @builtin_csa_parallel_loop_ to void ()*)() #1
  %3 = icmp slt i32 %1, %0
  br i1 %3, label %"4", label %"3.preheader"

"3.preheader":                                    ; preds = %entry
  %4 = sub i32 %1, %0
  %5 = add i32 %0, 1
  %6 = mul i32 %4, %5
  %7 = shl i32 %0, 1
  %8 = add i32 %1, -1
  %9 = sub i32 %8, %0
  %10 = zext i32 %9 to i33
  %11 = zext i32 %4 to i33
  %12 = mul i33 %10, %11
  %13 = lshr i33 %12, 1
  %14 = trunc i33 %13 to i32
  %15 = add i32 %1, %6
  %16 = add i32 %15, %7
  %17 = add i32 %16, %14
  br label %"4"

"4":                                              ; preds = %"3.preheader", %entry
  %18 = phi i32 [ %2, %entry ], [ %17, %"3.preheader" ]
  ret i32 %18
}

declare void @builtin_csa_parallel_loop_(...) local_unnamed_addr

attributes #0 = { nounwind uwtable "no-frame-pointer-elim-non-leaf"="false" }
attributes #1 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"GCC: (GNU) 4.8.2 LLVM: 4.0.0svn"}
!1 = !{!2, !2, i64 0}
!2 = !{!"alias set 7: integer(kind=4)", !3, i64 0}
!3 = distinct !{!3}
