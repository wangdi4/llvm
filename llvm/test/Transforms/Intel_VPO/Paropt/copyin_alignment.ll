; RUN: opt -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes="vpo-paropt" -S %s | FileCheck %s

@nder = dso_local thread_private global i32 0, align 4
@derpar_ = common unnamed_addr global [9 x i8] zeroinitializer, align 32
@der_ = common unnamed_addr global [8 x i8] zeroinitializer, align 32

;1. C Code:
; int nder;
; #pragma omp threadprivate(nder)
; void foo() {
; #pragma omp parallel copyin(nder)
;       nder = 12;
; }
define dso_local void @_foo_globalvar() local_unnamed_addr #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.COPYIN"(i32* @nder) ]
  br label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %entry
  store i32 12, i32* @nder, align 4, !tbaa !2
  br label %DIR.OMP.END.PARALLEL.4

DIR.OMP.END.PARALLEL.4:                           ; preds = %DIR.OMP.END.PARALLEL.3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}
; CHECK-LABEL:define internal void @_foo_globalvar{{.*}}.split
; CHECK: call void @llvm.memcpy{{.*}}(i8* align 4

;2. Fortran Code:
;      SUBROUTINE FOO
;      CHARACTER X
;      INTEGER NDER, BBB
;      COMMON/DERPAR/X,NDER,BBB
;!$omp threadprivate(/DERPAR/)
;!$OMP PARALLEL copyin(NDER)
;           NDER=12
;!$OMP END PARALLEL
;      END

define void @foo_bitcast_gep() local_unnamed_addr #0 {
alloca_0:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.COPYIN"(i32* bitcast (i8* getelementptr inbounds ([9 x i8], [9 x i8]* @derpar_, i64 0, i64 1) to i32*)) ]
  br label %bb3

bb3:                                              ; preds = %alloca_0
  store i32 12, i32* bitcast (i8* getelementptr inbounds ([9 x i8], [9 x i8]* @derpar_, i64 0, i64 1) to i32*), align 1
  br label %DIR.OMP.END.PARALLEL.4

DIR.OMP.END.PARALLEL.4:                           ; preds = %bb3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}
; CHECK-LABEL:define internal void @foo_bitcast_gep{{.*}}.split(
; CHECK: call void @llvm.memcpy{{.*}}(i8* align 1

;3. Fortran Code:
;      SUBROUTINE FOO
;      CHARACTER X
;      INTEGER APriv, B
;      COMMON/DER/APriv, B
;!$omp threadprivate(/DER/)
;!$OMP PARALLEL copyin(APriv)
;           APriv=12
;!$OMP END PARALLEL
;      END

define void @foo_bitcast() local_unnamed_addr #0 {
alloca_0:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.COPYIN"(i32* bitcast ([8 x i8]* @der_ to i32*)) ]
  br label %bb3

bb3:                                              ; preds = %alloca_0
  store i32 12, i32* bitcast ([8 x i8]* @der_ to i32*), align 32
  br label %DIR.OMP.END.PARALLEL.4

DIR.OMP.END.PARALLEL.4:                           ; preds = %bb3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}
; CHECK-LABEL:define internal void @foo_bitcast{{.*}}.split(
; CHECK: call void @llvm.memcpy{{.*}}(i8* align 32

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}

