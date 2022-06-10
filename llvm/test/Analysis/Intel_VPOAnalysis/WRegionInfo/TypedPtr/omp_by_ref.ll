; RUN: opt -enable-new-pm=0 -vpo-wrncollection -analyze %s | FileCheck %s
; RUN: opt -passes='function(print<vpo-wrncollection>)' -disable-output %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CMPLRLLVM_901: BY-REF support
; In the test below, bbb is passed by reference, so its representation
; in the firstprivate clause has the BYREF modifier, resulting
; in a tag name like this: "QUAL.OMP.FIRSTPRIVATE:BYREF"
;
; This test verifies that WRN construction parses this tag name
; and represents the BYREF property correctly. Given the test below
;
; void foo(int aaa, int &bbb) {
;   #pragma omp parallel firstprivate(aaa,bbb)
;     bbb = aaa + 123;
; }
;
; the WRN graph dump should result in something like this:
;
; BEGIN PARALLEL ID=1 {
;
;   IF_EXPR: UNSPECIFIED
;   NUM_THREADS: UNSPECIFIED
;   DEFAULT: UNSPECIFIED
;   PROCBIND: UNSPECIFIED
;   SHARED clause: UNSPECIFIED
;   PRIVATE clause: UNSPECIFIED
;   FIRSTPRIVATE clause (size=2): (i32* %aaa.addr) BYREF(i32* %0) <- CHECK THIS
;   REDUCTION clause: UNSPECIFIED
;   COPYIN clause: UNSPECIFIED
;
;   EntryBB: DIR.OMP.PARALLEL.1
;   ExitBB: DIR.OMP.END.PARALLEL.3
;
; } END PARALLEL ID=1

; CHECK: FIRSTPRIVATE clause (size=2):{{.*}} BYREF(i32* {{.*}})

; Function Attrs: nounwind uwtable
define dso_local void @_Z3fooiRi(i32 %aaa, i32* dereferenceable(4) %bbb) #0 {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca i32*, align 8
  store i32 %aaa, i32* %aaa.addr, align 4, !tbaa !2
  store i32* %bbb, i32** %bbb.addr, align 8, !tbaa !6
  %0 = load i32*, i32** %bbb.addr, align 8, !tbaa !6
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %aaa.addr), "QUAL.OMP.FIRSTPRIVATE:BYREF"(i32* %0) ]
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  %2 = load i32, i32* %aaa.addr, align 4, !tbaa !2
  %add = add nsw i32 %2, 123
  %3 = load i32*, i32** %bbb.addr, align 8, !tbaa !6
  store i32 %add, i32* %3, align 4, !tbaa !2
  br label %DIR.OMP.END.PARALLEL.3

DIR.OMP.END.PARALLEL.3:                           ; preds = %DIR.OMP.PARALLEL.2
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.4

DIR.OMP.END.PARALLEL.4:                           ; preds = %DIR.OMP.END.PARALLEL.3
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 5be402b1c6a5f2f334635d9257915bd7c3f4b734) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm d4632b6348b82823479bab9a392d33ce89e0f8e3)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"any pointer", !4, i64 0}
