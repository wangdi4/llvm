; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; This file tests the implementation of omp task and omp taskwait
; int fib(int n)
; {
;  int i, j;
;  if (n<2)
;    return n;
;  else
;    {
;       #pragma omp task shared(i) firstprivate(n)
;       i=fib(n-1);
;
;       #pragma omp task shared(j) firstprivate(n)
;       j=fib(n-2);
;
;       #pragma omp taskwait
;       return i+j;
;    }
;}


target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [14 x i8] c"fib(%d) = %d\0A\00", align 1

; Function Attrs: nounwind uwtable
define i32 @fib(i32 %n) #0 {
entry:
  %retval = alloca i32, align 4
  %n.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %cleanup.dest.slot = alloca i32
  store i32 %n, i32* %n.addr, align 4, !tbaa !1
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start(i64 4, i8* %0) #2
  %1 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start(i64 4, i8* %1) #2
  %2 = load i32, i32* %n.addr, align 4, !tbaa !1
  %cmp = icmp slt i32 %2, 2
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %3 = load i32, i32* %n.addr, align 4, !tbaa !1
  store i32 %3, i32* %retval, align 4
  store i32 1, i32* %cleanup.dest.slot, align 4
  br label %cleanup

if.else:                                          ; preds = %entry
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.SHARED"(i32* %i), "QUAL.OMP.FIRSTPRIVATE"(i32* %n.addr) ]
  %5 = load i32, i32* %n.addr, align 4, !tbaa !1
  %sub = sub nsw i32 %5, 1
  %call = call i32 @fib(i32 %sub)
  store i32 %call, i32* %i, align 4, !tbaa !1
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TASK"() ]
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.SHARED"(i32* %j), "QUAL.OMP.FIRSTPRIVATE"(i32* %n.addr) ]
  %7 = load i32, i32* %n.addr, align 4, !tbaa !1
  %sub1 = sub nsw i32 %7, 2
  %call2 = call i32 @fib(i32 %sub1)
  store i32 %call2, i32* %j, align 4, !tbaa !1
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TASK"() ]
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKWAIT"() ]
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TASKWAIT"() ]
  %9 = load i32, i32* %i, align 4, !tbaa !1
  %10 = load i32, i32* %j, align 4, !tbaa !1
  %add = add nsw i32 %9, %10
  store i32 %add, i32* %retval, align 4
  store i32 1, i32* %cleanup.dest.slot, align 4
  br label %cleanup

cleanup:                                          ; preds = %if.else, %if.then
  %11 = bitcast i32* %j to i8*
  call void @llvm.lifetime.end(i64 4, i8* %11) #2
  %12 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end(i64 4, i8* %12) #2
  %13 = load i32, i32* %retval, align 4
  ret i32 %13
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
entry:
  %n = alloca i32, align 4
  %0 = bitcast i32* %n to i8*
  call void @llvm.lifetime.start(i64 4, i8* %0) #2
  store i32 10, i32* %n, align 4, !tbaa !1
  call void @omp_set_dynamic(i32 0)
  call void @omp_set_num_threads(i32 4)
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(i32* %n), "QUAL.OMP.SHARED"(i32* %n) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"() ]
  %3 = load i32, i32* %n, align 4, !tbaa !1
  %4 = load i32, i32* %n, align 4, !tbaa !1
  %call = call i32 @fib(i32 %4)
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str, i32 0, i32 0), i32 %3, i32 %call)
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SINGLE"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  %5 = bitcast i32* %n to i8*
  call void @llvm.lifetime.end(i64 4, i8* %5) #2
  ret i32 0
}

declare void @omp_set_dynamic(i32) #3

declare void @omp_set_num_threads(i32) #3

declare i32 @printf(i8*, ...) #3

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (cfe/trunk)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}

; CHECK:  %{{.*}} = call i8* @__kmpc_omp_task_alloc({{.*}})
; CHECK:  call void @__kmpc_omp_task({{.*}})
; CHECK:  call void @__kmpc_omp_taskwait({{.*}})
; sizeof_kmp_task_t and sizeof_shareds arguments are 8-byte integers for 64-bit target.
; CHECK:  declare i8* @__kmpc_omp_task_alloc({{[^,]+}}, i32, i32, i64, i64, i32 (i32, i8*)*)
