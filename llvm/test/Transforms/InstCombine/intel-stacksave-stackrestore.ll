; RUN: opt -instcombine -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes=instcombine -S < %s 2>&1 | FileCheck %s

; Original code (with manually inserted stacksave/stackrestore):
; #include <string.h>
; int foo() {
;   int a[100];
;
;   memset(a, 0, 100 * sizeof(int));
;   return a[7];
; }

; CMPLRLLVM-26364: check that instcombine is able to remove
;                  stacksave/stackrestore calls.

; CHECK-NOT: call{{.*}}stacksave
; CHECK-NOT: call{{.*}}stackrestore

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @foo() #0 {
entry:
  %a = alloca [100 x i32], align 16
  %savedstack = tail call i8* @llvm.stacksave()
  %0 = bitcast [100 x i32]* %a to i8*
  call void @llvm.lifetime.start.p0i8(i64 400, i8* %0) #3
  %arraydecay = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 0
  %1 = bitcast i32* %arraydecay to i8*
  call void @llvm.memset.p0i8.i64(i8* align 16 %1, i8 0, i64 400, i1 false)
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 7, !intel-tbaa !2
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %3 = bitcast [100 x i32]* %a to i8*
  call void @llvm.lifetime.end.p0i8(i64 400, i8* %3) #3
  tail call void @llvm.stackrestore(i8* %savedstack)
  ret i32 %2
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nofree nosync nounwind willreturn
declare i8* @llvm.stacksave() #4

; Function Attrs: nofree nosync nounwind willreturn
declare void @llvm.stackrestore(i8*) #4

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { argmemonly nofree nosync nounwind willreturn writeonly }
attributes #3 = { nounwind }
attributes #4 = { nofree nosync nounwind willreturn }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.2.0.YYYYMMDD)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
