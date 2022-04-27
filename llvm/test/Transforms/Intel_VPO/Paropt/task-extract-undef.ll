; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s

; CHECK: @main() local
; CHECK: %x = alloca
; CHECK: %task{{.*}}priv{{.*}} = alloca

; The %x alloca must not be moved into the parallel region, as the memory
; is written asynchronously by the tasks and must not be popped off the
; stack until all the tasks end (at the end of the program)

target triple = "x86_64-unknown-linux-gnu"

@gg = dso_local global i32 4, align 4

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %x = alloca i32, align 4
  %0 = bitcast i32* %x to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #2
  br label %DIR.OMP.PARALLEL.1.split

DIR.OMP.PARALLEL.1.split:                         ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(i32* @gg), "QUAL.OMP.SHARED"(i32* %x) ]
  br label %DIR.OMP.PARALLEL.2.split

DIR.OMP.PARALLEL.2.split:                         ; preds = %DIR.OMP.PARALLEL.1.split
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.SHARED"(i32* @gg), "QUAL.OMP.SHARED"(i32* %x) ]
  %3 = load volatile i32, i32* @gg, align 4, !tbaa !2
  store i32 %3, i32* %x, align 4, !tbaa !2
  br label %DIR.OMP.END.TASK.4

DIR.OMP.END.TASK.4:                               ; preds = %DIR.OMP.PARALLEL.2.split
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TASK"() ]
  br label %DIR.OMP.END.TASK.5

DIR.OMP.END.TASK.5:                               ; preds = %DIR.OMP.END.TASK.4
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #2
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
