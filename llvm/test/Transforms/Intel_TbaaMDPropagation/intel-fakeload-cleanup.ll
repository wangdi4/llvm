; RUN: opt -passes=cleanup-fakeloads -S < %s | FileCheck %s

%struct.S = type { [4 x i32], [4 x i32] }

$_ZN1S4getaEi = comdat any

$_ZN1S4getbEi = comdat any

; Function Attrs: nounwind uwtable
define ptr @test(ptr %this, i32 %i) {
entry:
  %this.addr = alloca ptr, align 8
  %i.addr = alloca i32, align 4
  store ptr %this, ptr %this.addr, align 8, !tbaa !6
  store i32 %i, ptr %i.addr, align 4, !tbaa !4
  %this1 = load ptr, ptr %this.addr, align 8
  %0 = load i32, ptr %i.addr, align 4, !tbaa !4
  %idxprom = sext i32 %0 to i64
  %arrayidx = getelementptr inbounds [4 x i32], ptr %this1, i64 0, i64 %idxprom
  %1 = call ptr @llvm.intel.fakeload.p0(ptr %arrayidx, metadata !8)
  ret ptr %1
}

; CHECK-NOT: call ptr @llvm.intel.fakeload

; Function Attrs: nounwind
declare ptr @llvm.intel.fakeload.p0(ptr, metadata)

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 17977)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C++ TBAA"}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !2, i64 0}
!6 = !{!7, !7, i64 0}
!7 = !{!"pointer@_ZTSP1S", !2, i64 0}
!8 = !{!9, !5, i64 0}
!9 = !{!"struct@_ZTS1S", !10, i64 0, !10, i64 16}
!10 = !{!"array@_ZTSA4_i", !5, i64 0}
!11 = !{!9, !5, i64 16}

