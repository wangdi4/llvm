; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Check whether the compiler generates the correct map type for struct member.
; struct S1 {
;   double *d;
; };
;
; void foo(S1 *ps1)
; {
;   #pragma omp target map(to:ps1->d[1:100])
;   {
;     ps1->d[50] = 10;
;   }
; }

target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-mic"

%struct.S1 = type { double* }

@"@tid.addr" = external global i32

; Function Attrs: nounwind uwtable
define dso_local void @_Z3fooP2S1(%struct.S1* %ps1) #0 {
entry:
  %d = getelementptr inbounds %struct.S1, %struct.S1* %ps1, i32 0, i32 0, !intel-tbaa !7
  %0 = load double*, double** %d, align 8, !tbaa !7
  %1 = getelementptr double*, double** %d, i32 1
  %2 = bitcast double** %d to i8*
  %3 = bitcast double** %1 to i8*
  %4 = ptrtoint i8* %3 to i64
  %5 = ptrtoint i8* %2 to i64
  %6 = sub i64 %4, %5
  %7 = sdiv exact i64 %6, ptrtoint (i8* getelementptr (i8, i8* null, i32 1) to i64)
  %arrayidx = getelementptr inbounds double, double* %0, i64 1
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  %ret = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TO:AGGRHEAD"(%struct.S1* %ps1, double** %d, i64 %7), "QUAL.OMP.MAP.TO:AGGR"(double** %d, double* %arrayidx, i64 800) ]
  %d2 = getelementptr inbounds %struct.S1, %struct.S1* %ps1, i32 0, i32 0, !intel-tbaa !7
  %d3 = load double*, double** %d2, align 8, !tbaa !7
  %arrayidx3 = getelementptr inbounds double, double* %d3, i64 50
  store double 1.000000e+01, double* %arrayidx3, align 8, !tbaa !10
  br label %DIR.OMP.END.TARGET.3

DIR.OMP.END.TARGET.3:                             ; preds = %DIR.OMP.TARGET.1
  call void @llvm.directive.region.exit(token %ret) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 47, i32 -1941798063, !"_Z3fooP2S1", i32 8, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 8.0.0"}
!3 = !{!4, !4, i64 0}
!4 = !{!"unspecified pointer", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !9, i64 0}
!8 = !{!"struct@_ZTS2S1", !9, i64 0}
!9 = !{!"pointer@_ZTSPd", !5, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"double", !5, i64 0}

; CHECK: @.offload_maptypes = private unnamed_addr constant [2 x i64] [i64 32, i64 281474976710673]
