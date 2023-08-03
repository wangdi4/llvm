; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-paropt-prepare-disable-offload -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -vpo-paropt-prepare-disable-offload -S %s | FileCheck %s

; Source code:
; int main() {
;   int x = 100;
;   int y = 200;
;   #pragma omp target private(x) firstprivate(y)
;   {
;     x = 888;
;     y = 999;
;   }
;   printf("x=%d y=%d\n", x, y);  // x=100 y=200 even if offload is disabled
;   return 0;
; }
;
; Check that [FIRST]PRIVATE semantics are maintained even when offload is disabled.
; In other words, the expected output is x=100 y=200. Just ignoring the TARGET pragma
; results in bad IR because the original "x" and "y" gets overwritten.
;
;   store i32 888, i32* %x
;   store i32 999, i32* %y
;   %1 = load i32, i32* %x
;   %2 = load i32, i32* %y
;   %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i64 0, i64 0), i32 %1)
;
; Correct behavior: we still need to create the private copies and store into
; them instead of overwriting the original %x and %y:
;
;   %y.fpriv = alloca i32
;   %x.priv = alloca i32
;   store i32 888, i32* %x.priv
;   store i32 999, i32* %y.fpriv
;   %1 = load i32, i32* %x
;   %2 = load i32, i32* %y
;   %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str, i64 0, i64 0), i32 %1, i32 %2)

; CHECK-DAG: %y.fpriv = alloca i32
; CHECK-DAG: %x.priv = alloca i32
; CHECK: store i32 888, ptr %x.priv
; CHECK: store i32 999, ptr %y.fpriv

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [11 x i8] c"x=%d y=%d\0A\00", align 1

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %x = alloca i32, align 4
  %y = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  store i32 100, ptr %x, align 4
  store i32 200, ptr %y, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %x, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y, i32 0, i32 1) ]

  store i32 888, ptr %x, align 4
  store i32 999, ptr %y, align 4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  %1 = load i32, ptr %x, align 4
  %2 = load i32, ptr %y, align 4
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %1, i32 noundef %2)
  ret i32 0
}

declare token @llvm.directive.region.entry() 

declare void @llvm.directive.region.exit(token) 

declare dso_local i32 @printf(ptr noundef, ...) 

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 64773, i32 3825464, !"_Z4main", i32 6, i32 0, i32 0, i32 0}

