; RUN: opt -opaque-pointers -vpo-restore-operands -S %s | FileCheck %s -check-prefixes=RESTR,ALL
; RUN: opt -opaque-pointers -passes='function(vpo-restore-operands)' -S %s | FileCheck %s -check-prefixes=RESTR,ALL
; RUN: opt -opaque-pointers -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefixes=TFORM,ALL
; RUN: opt -opaque-pointers -passes='function(vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefixes=TFORM,ALL

; Test src:

;  int y;
;  int b() {
;  #pragma omp parallel firstprivate(a)
;    y = 111;
;  }
;  int c() { b(); }

; The IR is the output of inliner, which inserts lifetime begin/end intrinsics
; for temp vars generated in prepare pass.

; Additional lifetime end markers were added for %y.addr.i to make sure we
; can handle deletion of multiple markers for the same operand addr.

; ALL-NOT:       %y.addr.i

; RESTR-COUNT-1: call void @llvm.lifetime.start.p0
; RESTR-COUNT-1: call void @llvm.lifetime.end.p0

; TFORM-NOT:     %end.dir.temp.i
; TFORM-NOT:     call void @llvm.lifetime.start.p0
; TFORM-NOT:     call void @llvm.lifetime.end.p0

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@y = dso_local global i32 0, align 4

; Function Attrs: nounwind uwtable
define dso_local i32 @c() local_unnamed_addr #0 {
entry:
  %y.addr.i = alloca ptr, align 8
  %end.dir.temp.i = alloca i1, align 1
  call void @llvm.lifetime.start.p0(i64 8, ptr %y.addr.i)
  call void @llvm.lifetime.start.p0(i64 1, ptr %end.dir.temp.i)
  store ptr @y, ptr %y.addr.i, align 8

  %0 = call token @llvm.directive.region.entry() #1 [ "DIR.OMP.PARALLEL"(),
  "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @y, i32 0, i32 1),
  "QUAL.OMP.OPERAND.ADDR"(ptr @y, ptr %y.addr.i),
  "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp.i) ]

  %temp.load.i = load volatile i1, ptr %end.dir.temp.i, align 1
  br i1 %temp.load.i, label %b.exit, label %DIR.OMP.PARALLEL.3.i

DIR.OMP.PARALLEL.3.i:                             ; preds = %entry
  %y.i = load volatile ptr, ptr %y.addr.i, align 8
  store i32 111, ptr %y.i, align 4, !tbaa !4
  br label %b.exit

b.exit:                                           ; preds = %entry, %DIR.OMP.PARALLEL.3.i
  call void @llvm.directive.region.exit(token %0) #1 [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.lifetime.end.p0(i64 8, ptr %y.addr.i)
  call void @llvm.lifetime.end.p0(i64 8, ptr %y.addr.i)
  call void @llvm.lifetime.end.p0(i64 8, ptr %y.addr.i)
  call void @llvm.lifetime.end.p0(i64 1, ptr %end.dir.temp.i)
  ret i32 undef
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nofree nosync nounwind willreturn }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{!"clang version 13.0.0"}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
