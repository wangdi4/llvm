; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Check whether the compiler generates the correct code for firstprivate global.
; 
; #pragma omp declare target
; int pvtPtr;
; #pragma omp end declare target
; int main() {
;   scanf("%d", &pvtPtr);
;   int out;
;
;   #pragma omp target firstprivate(pvtPtr) map(from: out)
;   {
;      out = pvtPtr + 1;
;   }
;   printf("%d\n", out);
; }

; CHECK: %pvtPtr.fpriv = alloca i32

target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-unknown-linux-gnu"

@pvtPtr = dso_local target_declare global i32 0, align 4
@.str = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@.str.1 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 0, ptr @.omp_offloading.requires_reg, ptr null }]

define dso_local noundef i32 @main() {
entry:
  %out = alloca i32, align 4
  %call = call i32 (ptr, ...) @__isoc99_scanf(ptr noundef @.str, ptr noundef @pvtPtr)
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @pvtPtr, i32 0, i32 1),
    "QUAL.OMP.MAP.FROM"(ptr %out, ptr %out, i64 4, i64 34, ptr null, ptr null) ] ; MAP type: 34 = 0x22 = TARGET_PARAM (0x20) | FROM (0x2)

  %1 = load i32, ptr @pvtPtr, align 4
  %add = add nsw i32 %1, 1
  store i32 %add, ptr %out, align 4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  %2 = load i32, ptr %out, align 4
  %call1 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef %2)
  ret i32 0
}

declare dso_local i32 @__isoc99_scanf(ptr noundef, ...)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local i32 @printf(ptr noundef, ...)

define internal void @.omp_offloading.requires_reg() #3 section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

declare void @__tgt_register_requires(i64)

!omp_offload.info = !{!0, !1}
!llvm.module.flags = !{!2, !3, !4, !5}

!0 = !{i32 0, i32 64773, i32 3828821, !"_Z4main", i32 10, i32 0, i32 1, i32 0}
!1 = !{i32 1, !"_Z6pvtPtr", i32 0, i32 0, ptr @pvtPtr}
!2 = !{i32 1, !"wchar_size", i32 4}
!3 = !{i32 7, !"openmp", i32 51}
!4 = !{i32 7, !"uwtable", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
