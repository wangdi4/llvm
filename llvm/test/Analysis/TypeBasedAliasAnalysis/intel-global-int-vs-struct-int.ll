; RUN: opt -tbaa -aa-eval -evaluate-aa-metadata -print-may-aliases -disable-output < %s 2>&1 | FileCheck %s

; CHECK:   MayAlias:   %1 = load i32, i32* %a, align 4, !tbaa !6 <->   store i32 %1, i32* @h, align 4, !tbaa !9
; CHECK:   MayAlias:   %1 = load i32, i32* %a, align 4, !tbaa !6 <->   store i32 5, i32* @g, align 4, !tbaa !9
; CHECK:   MayAlias:   %2 = load i32, i32* %a, align 4, !tbaa !6 <->   store i32 %1, i32* @h, align 4, !tbaa !9
; CHECK:   MayAlias:   %2 = load i32, i32* %a, align 4, !tbaa !6 <->   store i32 5, i32* @g, align 4, !tbaa !9


%struct.T = type { i32, i32, i32 }

@g = dso_local global i32 0, align 4
@h = dso_local global i32 0, align 4
@p = dso_local global %struct.T* null, align 8

; Function Attrs: nounwind uwtable
define dso_local i32 @foo() #0 {
entry:
  %0 = load %struct.T*, %struct.T** @p, align 8, !tbaa !2
  %a = getelementptr inbounds %struct.T, %struct.T* %0, i32 0, i32 0
  %1 = load i32, i32* %a, align 4, !tbaa !6
  store i32 %1, i32* @h, align 4, !tbaa !9
  store i32 5, i32* @g, align 4, !tbaa !9
  %2 = load i32, i32* %a, align 4, !tbaa !6
  ret i32 %2
}


!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"unspecified pointer", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !8, i64 0}
!7 = !{!"struct@_ZTS5T", !8, i64 0, !8, i64 4, !8, i64 8}
!8 = !{!"int", !4, i64 0}
!9 = !{!8, !8, i64 0}
