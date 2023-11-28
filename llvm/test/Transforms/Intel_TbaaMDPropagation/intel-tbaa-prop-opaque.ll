; RUN: opt -passes="tbaa-prop" -S %s | FileCheck %s

; Verify that TBAA propagation will propagate metadata from GEPs to memory
; instructions of the same result type as the GEP.

; CHECK: %c = getelementptr {{.*}} !intel-tbaa ![[MDS:[0-9]+]]
; CHECK: %a = getelementptr {{.*}} !intel-tbaa ![[MDL:[0-9]+]]
; CHECK: load i8, ptr %a, align 1, !tbaa ![[MDL]]
; CHECK-NOT: store i8 %val1, ptr %c, align 1, !tbaa ![[MDS]]
; CHECK: store i8 %val1, ptr %c, align 1, !tbaa !{{[0-9]+}}

target triple = "x86_64-unknown-linux-gnu"

%struct.d = type { %struct.anon }
%struct.anon = type { i8, i8 }

; Function Attrs: nounwind uwtable
define dso_local void @g() local_unnamed_addr {
entry:
  %h = alloca %struct.d, align 1
; GEP result is "struct" type and cannot be propagated to the i8 store
  %c = getelementptr inbounds %struct.d, ptr %h, i64 0, i32 0, !intel-tbaa !3
; GEP result is i8 type and can be propagated to the load
  %a = getelementptr inbounds %struct.d, ptr %h, i64 0, i32 0, i32 0, !intel-tbaa !10
  %val1 = load i8, ptr %a, align 1, !tbaa !10
  store i8 %val1, ptr %c, align 1, !tbaa !11
  ret void
}

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"struct@", !5, i64 0}
!5 = !{!"struct@", !6, i64 0, !6, i64 1}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !6, i64 0}
!10 = !{!4, !6, i64 0}
!11 = !{!6, !6, i64 0}
!12 = distinct !{!12, !13}
!13 = !{!"llvm.loop.mustprogress"}
