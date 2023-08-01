; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

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

; CHECK:  %{{.*}} = call i8* @__kmpc_omp_task_alloc({{.*}})
; CHECK:  call void @__kmpc_omp_task({{.*}})
; CHECK-NOT: call void @__kmpc_omp_wait_deps
; CHECK:  call void @__kmpc_omp_taskwait({{.*}})
; sizeof_kmp_task_t and sizeof_shareds arguments are 8-byte integers for 64-bit target.
; CHECK:  declare i8* @__kmpc_omp_task_alloc({{[^,]+}}, i32, i32, i64, i64, i32 (i32, i8*)*)

target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [14 x i8] c"fib(%d) = %d\0A\00", align 1

define i32 @fib(i32 %n) {
entry:
  %retval = alloca i32, align 4
  %n.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %cleanup.dest.slot = alloca i32
  store i32 %n, i32* %n.addr, align 4
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start(i64 4, i8* %0)
  %1 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start(i64 4, i8* %1)
  %2 = load i32, i32* %n.addr, align 4
  %cmp = icmp slt i32 %2, 2
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %3 = load i32, i32* %n.addr, align 4
  store i32 %3, i32* %retval, align 4
  store i32 1, i32* %cleanup.dest.slot, align 4
  br label %cleanup

if.else:                                          ; preds = %entry
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.SHARED"(i32* %i),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %n.addr) ]

  %5 = load i32, i32* %n.addr, align 4
  %sub = sub nsw i32 %5, 1
  %call = call i32 @fib(i32 %sub)
  store i32 %call, i32* %i, align 4
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TASK"() ]
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.SHARED"(i32* %j),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %n.addr) ]

  %7 = load i32, i32* %n.addr, align 4
  %sub1 = sub nsw i32 %7, 2
  %call2 = call i32 @fib(i32 %sub1)
  store i32 %call2, i32* %j, align 4
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TASK"() ]
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKWAIT"() ]
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TASKWAIT"() ]
  %9 = load i32, i32* %i, align 4
  %10 = load i32, i32* %j, align 4
  %add = add nsw i32 %9, %10
  store i32 %add, i32* %retval, align 4
  store i32 1, i32* %cleanup.dest.slot, align 4
  br label %cleanup

cleanup:                                          ; preds = %if.else, %if.then
  %11 = bitcast i32* %j to i8*
  call void @llvm.lifetime.end(i64 4, i8* %11)
  %12 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end(i64 4, i8* %12)
  %13 = load i32, i32* %retval, align 4
  ret i32 %13
}

declare void @llvm.lifetime.start(i64, i8* nocapture)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @llvm.lifetime.end(i64, i8* nocapture)

define i32 @main() {
entry:
  %n = alloca i32, align 4
  %0 = bitcast i32* %n to i8*
  call void @llvm.lifetime.start(i64 4, i8* %0)
  store i32 10, i32* %n, align 4
  call void @omp_set_dynamic(i32 0)
  call void @omp_set_num_threads(i32 4)
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED"(i32* %n),
    "QUAL.OMP.SHARED"(i32* %n) ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"() ]
  %3 = load i32, i32* %n, align 4
  %4 = load i32, i32* %n, align 4
  %call = call i32 @fib(i32 %4)
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @.str, i32 0, i32 0), i32 %3, i32 %call)
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SINGLE"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  %5 = bitcast i32* %n to i8*
  call void @llvm.lifetime.end(i64 4, i8* %5)
  ret i32 0
}

declare void @omp_set_dynamic(i32)
declare void @omp_set_num_threads(i32)
declare i32 @printf(i8*, ...)
