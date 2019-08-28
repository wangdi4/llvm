; RUN: opt <%s -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -pass-remarks-output=%t -S
; RUN: FileCheck --input-file %t %s
; RUN: opt <%s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -pass-remarks-output=%t -S
; RUN: FileCheck --input-file %t %s

; Test to chack that opt-report messages are printed for VPO OpenMP.

;
; #include <stdio.h>
; #include <omp.h>
;
; void foo(int *a, int *b, int *c) {
;   int i;
; #pragma omp parallel
;   {
;     a[i] = b[i] + c[i];
;
; #pragma omp master
;     printf("tid = %d\n", omp_get_thread_num());
;   }
; }
;
; CHECK: Pass:{{[ ]*}}openmp
; CHECK: Construct:{{[ ]*}}master
; CHECK: String:{{[ ]*}}' construct transformed'
; CHECK: Pass:{{[ ]*}}openmp
; CHECK: Construct:{{[ ]*}}parallel
; CHECK: String:{{[ ]*}}' construct transformed'

; ModuleID = 'par.c'
source_filename = "par.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [10 x i8] c"tid = %d\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(i32* %a, i32* %b, i32* %c) #0 {
entry:
  %a.addr = alloca i32*, align 8
  %b.addr = alloca i32*, align 8
  %c.addr = alloca i32*, align 8
  %i = alloca i32, align 4
  store i32* %a, i32** %a.addr, align 8
  store i32* %b, i32** %b.addr, align 8
  store i32* %c, i32** %c.addr, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(i32** %b.addr), "QUAL.OMP.SHARED"(i32* %i), "QUAL.OMP.SHARED"(i32** %c.addr), "QUAL.OMP.SHARED"(i32** %a.addr) ]
  %1 = load i32*, i32** %b.addr, align 8
  %2 = load i32, i32* %i, align 4
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds i32, i32* %1, i64 %idxprom
  %3 = load i32, i32* %arrayidx, align 4
  %4 = load i32*, i32** %c.addr, align 8
  %5 = load i32, i32* %i, align 4
  %idxprom1 = sext i32 %5 to i64
  %arrayidx2 = getelementptr inbounds i32, i32* %4, i64 %idxprom1
  %6 = load i32, i32* %arrayidx2, align 4
  %add = add nsw i32 %3, %6
  %7 = load i32*, i32** %a.addr, align 8
  %8 = load i32, i32* %i, align 4
  %idxprom3 = sext i32 %8 to i64
  %arrayidx4 = getelementptr inbounds i32, i32* %7, i64 %idxprom3
  store i32 %add, i32* %arrayidx4, align 4
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASTER"() ]
  fence acquire
  %call = call i32 @omp_get_thread_num() #1
  %call5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str, i64 0, i64 0), i32 %call)
  fence release
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.MASTER"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

; Function Attrs: nounwind
declare dso_local i32 @omp_get_thread_num() #3

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
