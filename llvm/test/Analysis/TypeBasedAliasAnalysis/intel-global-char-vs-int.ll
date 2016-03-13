; RUN: opt -tbaa -gvn -S < %s | FileCheck %s

; The GVN is expected to remove the redundant load of 
; the parameter dereference using TBAA information 
; and global scalar information.

; CHECK: @foo
; CHECK:      entry:
; CHECK-NEXT:   store i32 10, i32* %s, align 4, !tbaa !1
; CHECK-NEXT:   store i8 12, i8* @a, align 1, !tbaa !5
; CHECK-NEXT:   ret i32 10

@a = common global i8 0, align 1

; Function Attrs: nounwind uwtable
define i32 @foo(i32* nocapture %s) #0 {
entry:
  store i32 10, i32* %s, align 4, !tbaa !1
  store i8 12, i8* @a, align 1, !tbaa !5
  %0 = load i32, i32* %s, align 4, !tbaa !1
  ret i32 %0
}


!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1702)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!3, !3, i64 0}
