; RUN: opt -cleanup-fakeloads -S < %s | FileCheck %s

%struct.S = type { [4 x i32], [4 x i32] }

$_ZN1S4getaEi = comdat any

$_ZN1S4getbEi = comdat any

; Function Attrs: nounwind uwtable
define i32* @test(%struct.S* %this, i32 %i) {
entry:
  %this.addr = alloca %struct.S*, align 8
  %i.addr = alloca i32, align 4
  store %struct.S* %this, %struct.S** %this.addr, align 8, !tbaa !6
  store i32 %i, i32* %i.addr, align 4, !tbaa !4
  %this1 = load %struct.S*, %struct.S** %this.addr, align 8
  %0 = load i32, i32* %i.addr, align 4, !tbaa !4
  %idxprom = sext i32 %0 to i64
  %a = getelementptr inbounds %struct.S, %struct.S* %this1, i32 0, i32 0
  %arrayidx = getelementptr inbounds [4 x i32], [4 x i32]* %a, i64 0, i64 %idxprom
  %1 = call i32* @llvm.intel.fakeload.p0i32(i32* %arrayidx, metadata !8) 
  ret i32* %1
}

; CHECK-NOT: call i32* @llvm.intel.fakeload

; Function Attrs: nounwind
declare i32* @llvm.intel.fakeload.p0i32(i32*, metadata) 


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

