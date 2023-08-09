; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes="vpo-paropt" -S %s | FileCheck %s

@nder = dso_local thread_private global i32 0, align 4
@derpar_ = common thread_private unnamed_addr global [9 x i8] zeroinitializer, align 32, !llfort.type_idx !0
@der_ = common thread_private unnamed_addr global [8 x i8] zeroinitializer, align 32, !llfort.type_idx !1

; 1. C Code:
;
; int nder;
; #pragma omp threadprivate(nder)
; void foo_globalvar() {
; #pragma omp parallel copyin(nder)
;       nder = 12;
; }

; Function Attrs: nounwind uwtable
define dso_local void @foo_globalvar() #0 {
entry:
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %entry
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.COPYIN:TYPED"(ptr @nder, i32 0, i32 1) ]
  br label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %DIR.OMP.PARALLEL.2
  store i32 12, ptr @nder, align 4, !tbaa !4
  br label %DIR.OMP.END.PARALLEL.4

DIR.OMP.END.PARALLEL.4:                           ; preds = %DIR.OMP.PARALLEL.3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.5

DIR.OMP.END.PARALLEL.5:                           ; preds = %DIR.OMP.END.PARALLEL.4
  ret void
}

; CHECK-LABEL:define internal void @foo_globalvar.{{.*}}
; CHECK: call void @llvm.memcpy{{.*}}(ptr align 4

; 2. Fortran Code:
;
;      SUBROUTINE FOO_GEP
;      CHARACTER X
;      INTEGER NDER, BBB
;      COMMON/DERPAR/X,NDER,BBB
;!$omp threadprivate(/DERPAR/)
;!$OMP PARALLEL copyin(NDER)
;           NDER=12
;!$OMP END PARALLEL
;      END

; Function Attrs: nounwind uwtable
define void @foo_gep_() #0 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 16, !llfort.type_idx !3
  br label %bb_new2

bb_new2:                                          ; preds = %alloca_0
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %bb_new2
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.COPYIN:TYPED"(ptr getelementptr inbounds ([9 x i8], ptr @derpar_, i32 0, i64 1), i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr getelementptr inbounds ([9 x i8], ptr @derpar_, i32 0, i64 1), i32 0, i32 1) ]
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  store i32 12, ptr getelementptr inbounds ([9 x i8], ptr @derpar_, i32 0, i64 1), align 1, !tbaa !4
  br label %DIR.OMP.END.PARALLEL.3

DIR.OMP.END.PARALLEL.3:                           ; preds = %DIR.OMP.PARALLEL.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.4

DIR.OMP.END.PARALLEL.4:                           ; preds = %DIR.OMP.END.PARALLEL.3
  ret void
}

; CHECK-LABEL:define internal void @foo_gep_.{{.*}}(
; CHECK: call void @llvm.memcpy{{.*}}(ptr align 1

; 3. Fortran Code:
;
;      SUBROUTINE FOO
;      CHARACTER X
;      INTEGER APriv, B
;      COMMON/DER/APriv, B
;!$omp threadprivate(/DER/)
;!$OMP PARALLEL copyin(APriv)
;           APriv=12
;!$OMP END PARALLEL
;      END

define void @foo_() #0 {
alloca_1:
  %"$io_ctx" = alloca [8 x i64], align 16, !llfort.type_idx !8
  %"foo_$X" = alloca [1 x i8], align 8, !llfort.type_idx !9
  br label %bb_new5

bb_new5:                                          ; preds = %alloca_1
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %bb_new5
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.COPYIN:TYPED"(ptr @der_, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr @der_, i32 0, i32 1) ]
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  store i32 12, ptr @der_, align 32, !tbaa !10
  br label %DIR.OMP.END.PARALLEL.3

DIR.OMP.END.PARALLEL.3:                           ; preds = %DIR.OMP.PARALLEL.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.4

DIR.OMP.END.PARALLEL.4:                           ; preds = %DIR.OMP.END.PARALLEL.3
  ret void
}

; CHECK-LABEL:define internal void @foo_.{{.*}}(
; CHECK: call void @llvm.memcpy{{.*}}(ptr align 32

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!0 = !{i64 24}
!1 = !{i64 36}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i64 19}
!4 = !{!5, !5, i64 0}
!5 = !{!"ifx$unique_sym$1", !6, i64 0}
!6 = !{!"Generic Fortran Symbol", !7, i64 0}
!7 = !{!"ifx$root$1$foo_gep_"}
!8 = !{i64 35}
!9 = !{i64 38}
!10 = !{!11, !11, i64 0}
!11 = !{!"ifx$unique_sym$2", !12, i64 0}
!12 = !{!"Generic Fortran Symbol", !13, i64 0}
!13 = !{!"ifx$root$2$foo_"}
