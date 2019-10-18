; RUN: opt < %s  -vpo-paropt  -S | FileCheck %s

; CMPLRLLVM-1156: Uninitialized large array is passed as firstprivate to
; a task.
; Make sure that memcpy is used to copy the array into the task thunk.
; In the outlined task, we should not copy the whole array, but use the
; data directly from the thunk.

; Caller side
; CHECK: [[BREG0:%.+]] = bitcast {{.*}} %B to i8*
; CHECK: [[BREG:%.+]] = bitcast {{.*}} %B to i8*
; CHECK: call void @llvm.memcpy{{.*}}align 16 {{.*}}, {{.*}}[[BREG]]{{.*}} 4000000

; CHECK: [[TASKT:%.+]] = bitcast {{.*}} %taskt.shared.agg
; CHECK: call void @llvm.memcpy{{.*}}align 4 {{.*}}, {{.*}}[[TASKT]]{{.*}} 4000000

; Task side
; CHECK: define{{.*}}TASK
; CHECK-NOT: fpriv
; CHECK-NOT: memcpy

; Function Attrs: nounwind uwtable
define dso_local void @_Z3barv() local_unnamed_addr #0 {
entry:
  %B = alloca [1000000 x i32], align 16
  %0 = bitcast [1000000 x i32]* %B to i8*
  call void @llvm.lifetime.start.p0i8(i64 4000000, i8* nonnull %0) #2
  br label %DIR.OMP.TASK.1

DIR.OMP.TASK.1:                                   ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.FIRSTPRIVATE"([1000000 x i32]* %B) ]
  br label %DIR.OMP.TASK.11

DIR.OMP.TASK.11:                                  ; preds = %DIR.OMP.TASK.1
  %2 = call i8* @llvm.launder.invariant.group.p0i8(i8* nonnull %0)
  %arrayidx = getelementptr inbounds i8, i8* %2, i64 176
  %3 = bitcast i8* %arrayidx to i32*
  store i32 123, i32* %3, align 16, !tbaa !2
  br label %DIR.OMP.END.TASK.3

DIR.OMP.END.TASK.3:                               ; preds = %DIR.OMP.TASK.11
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASK"() ]
  br label %DIR.OMP.END.TASK.2

DIR.OMP.END.TASK.2:                               ; preds = %DIR.OMP.END.TASK.3
  call void @llvm.lifetime.end.p0i8(i64 4000000, i8* nonnull %0) #2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: inaccessiblememonly nounwind speculatable
declare i8* @llvm.launder.invariant.group.p0i8(i8*) #3

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
attributes #3 = { inaccessiblememonly nounwind speculatable }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000000_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
