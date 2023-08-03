; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
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
;
; CHECK: @.offload_maptypes = private unnamed_addr constant [2 x i64] [i64 32, i64 281474976710673]

target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%struct.S1 = type { ptr }

@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 0, ptr @.omp_offloading.requires_reg, ptr null }]

define dso_local void @_Z3fooP2S1(ptr noundef %ps1) {
entry:
  %ps1.addr = alloca ptr, align 8
  %ps1.map.ptr.tmp = alloca ptr, align 8
  store ptr %ps1, ptr %ps1.addr, align 8
  %0 = load ptr, ptr %ps1.addr, align 8
  %1 = load ptr, ptr %ps1.addr, align 8
  %2 = load ptr, ptr %ps1.addr, align 8
  %d = getelementptr inbounds %struct.S1, ptr %2, i32 0, i32 0
  %3 = load ptr, ptr %ps1.addr, align 8
  %d1 = getelementptr inbounds %struct.S1, ptr %3, i32 0, i32 0
  %4 = load ptr, ptr %d1, align 8
  %arrayidx = getelementptr inbounds double, ptr %4, i64 1
  %5 = getelementptr ptr, ptr %d, i32 1
  %6 = ptrtoint ptr %5 to i64
  %7 = ptrtoint ptr %d to i64
  %8 = sub i64 %6, %7
  %9 = sdiv exact i64 %8, ptrtoint (ptr getelementptr (i8, ptr null, i32 1) to i64)
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr %1, ptr %d, i64 %9, i64 32, ptr null, ptr null), ; MAP type: 32 = 0x20 = TARGET_PARAM (0x20)
    "QUAL.OMP.MAP.TO:CHAIN"(ptr %d, ptr %arrayidx, i64 800, i64 281474976710673, ptr null, ptr null), ; MAP type: 281474976710673 = 0x1000000000011 = MEMBER_OF_1 (0x1000000000000) | PTR_AND_OBJ (0x10) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr %ps1.map.ptr.tmp, ptr null, i32 1) ]

  br label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %DIR.OMP.TARGET.2
  store ptr %1, ptr %ps1.map.ptr.tmp, align 8
  %11 = load ptr, ptr %ps1.map.ptr.tmp, align 8
  %d2 = getelementptr inbounds %struct.S1, ptr %11, i32 0, i32 0
  %12 = load ptr, ptr %d2, align 8
  %arrayidx3 = getelementptr inbounds double, ptr %12, i64 50
  store double 1.000000e+01, ptr %arrayidx3, align 8
  br label %DIR.OMP.END.TARGET.4

DIR.OMP.END.TARGET.4:                             ; preds = %DIR.OMP.TARGET.3
  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.TARGET"() ]

  br label %DIR.OMP.END.TARGET.5

DIR.OMP.END.TARGET.5:                             ; preds = %DIR.OMP.END.TARGET.4
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

define internal void @.omp_offloading.requires_reg()  section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

declare void @__tgt_register_requires(i64)

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{i32 0, i32 64773, i32 3828809, !"_Z3fooP2S1", i32 7, i32 0, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}

