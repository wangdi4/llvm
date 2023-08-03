; RUN: opt -passes=sroa -S < %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.MYSTRUCT1 = type { i32, ptr }
%struct.MYSTRUCT0 = type { i32, i32 }

; Check that predicate-opt-restruct metadata is propagated from %cache_info
; to its remaining load after SROA

; CHECK: %field1 = getelementptr inbounds %struct.MYSTRUCT1, ptr %arg, i32 0, i32 1
; CHECK-NEXT: load ptr, ptr %field1, align 8, {{.*}} !predicate-opt-restrict !6

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind uwtable
define dso_local i32 @foo(ptr noundef %arg) #0 {
entry:
  %arg.addr = alloca ptr, align 8
  %cache_info = alloca ptr, align 8, !predicate-opt-restrict !3
  store ptr %arg, ptr %arg.addr, align 8, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 8, ptr %cache_info) #2
  %0 = load ptr, ptr %arg.addr, align 8, !tbaa !4
  %field1 = getelementptr inbounds %struct.MYSTRUCT1, ptr %0, i32 0, i32 1, !intel-tbaa !8
  %1 = load ptr, ptr %field1, align 8, !tbaa !8
  store ptr %1, ptr %cache_info, align 8, !tbaa !12
  %2 = load ptr, ptr %cache_info, align 8, !tbaa !12
  %field0 = getelementptr inbounds %struct.MYSTRUCT0, ptr %2, i32 0, i32 0, !intel-tbaa !13
  %3 = load i32, ptr %field0, align 4, !tbaa !13, !alias.scope !15, !noalias !3
  call void @llvm.lifetime.end.p0(i64 8, ptr %cache_info) #2
  ret i32 %3
}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.x.0.YYYYMMDD)"}
!3 = !{}
!4 = !{!5, !5, i64 0}
!5 = !{!"pointer@_ZTSP9MYSTRUCT1", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !11, i64 8}
!9 = !{!"struct@", !10, i64 0, !11, i64 8}
!10 = !{!"int", !6, i64 0}
!11 = !{!"pointer@_ZTSP9MYSTRUCT0", !6, i64 0}
!12 = !{!11, !11, i64 0}
!13 = !{!14, !10, i64 0}
!14 = !{!"struct@", !10, i64 0, !10, i64 4}
!15 = !{!16}
!16 = distinct !{!16, !17, !"foo: cache_info"}
!17 = distinct !{!17, !"foo"}
