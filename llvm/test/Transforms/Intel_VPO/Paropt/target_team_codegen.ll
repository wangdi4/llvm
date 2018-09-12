; RUN: opt < %s -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-cfg-restructuring -vpo-paropt  -S | FileCheck %s
;
; It checks whether the construt omp target team is supported in the OMP
; codegen.
; void bar();
; void foo(int n) {
;   #pragma omp target
;   #pragma omp teams num_teams(n)
;   {
;     bar();
;   }
; }
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-mic"

@"@tid.addr" = external global i32

; Function Attrs: nounwind uwtable
define dso_local void @_Z3barv() #0 {
entry:
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @_Z3fooi(i32 %n) #1 {
entry:
  %n.addr = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4, !tbaa !2
  %0 = load i32, i32* %n.addr, align 4, !tbaa !2
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %n.addr) ], !omp_offload.entry !7
  br label %DIR.OMP.TEAMS.3

DIR.OMP.TEAMS.3:                                  ; preds = %DIR.OMP.TARGET.1
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.NUM_TEAMS"(i32 %0) ]
  call void @_Z3barv()
  br label %DIR.OMP.END.TEAMS.5

DIR.OMP.END.TEAMS.5:                              ; preds = %DIR.OMP.TEAMS.3
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TEAMS"() ]
  br label %DIR.OMP.END.TEAMS.6

DIR.OMP.END.TEAMS.6:                              ; preds = %DIR.OMP.END.TEAMS.5
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!omp_offload.info = !{!6}
!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 6d93f34e605c44d05e5c49346cf267f862c04f87) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm d9be553a568d4f571e0aa893701ea82bc05da651)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{i32 0, i32 54, i32 -698850821, !"_Z3fooi", i32 33, i32 0}
!7 = distinct !{i32 0}

; CHECK:  call i32 @__tgt_target_teams({{.*}})
