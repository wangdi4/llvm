; RUN: opt -tbaa -gvn -S < %s | FileCheck %s

; The GVN is expected to remove the redundant load of the 
; structure field dereference using the TBAA information 
; and global scalar information.

; CHECK: @foo
; CHECK:      entry:
; CHECK-NEXT:   %i = getelementptr inbounds %struct.T, %struct.T* %s, i64 0, i32 0
; CHECK-NEXT:   store i8 10, i8* %i, align 4, !tbaa !1
; CHECK-NEXT:   store i8 12, i8* @a, align 1, !tbaa !6
; CHECK-NEXT:   ret i8 10

%struct.T = type { i8, i32 }

@a = common global i8 0, align 1

; Function Attrs: nounwind uwtable
define signext i8 @foo(%struct.T* nocapture %s) #0 {
entry:
  %i = getelementptr inbounds %struct.T, %struct.T* %s, i64 0, i32 0
  store i8 10, i8* %i, align 4, !tbaa !1
  store i8 12, i8* @a, align 1, !tbaa !6
  %0 = load i8, i8* %i, align 4, !tbaa !1
  ret i8 %0
}

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1702)"}
!1 = !{!2, !3, i64 0}
!2 = !{!"", !3, i64 0, !5, i64 4}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!"int", !3, i64 0}
!6 = !{!3, !3, i64 0}
