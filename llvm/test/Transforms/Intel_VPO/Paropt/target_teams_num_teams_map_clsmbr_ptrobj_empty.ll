; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Check that the map-type of %this1 is updated to 32 from 0, and it's
; passed to the kernel.
; 32 (PARAM) for %this1, 281474976710674 (MEMBER_OF_1 | PTR_AND_OBJ | FROM) for %y, 288 (PARAM | LITERAL) is for %x.
; CHECK: @.offload_maptypes = {{.*}} [i64 32, i64 281474976710674, i64 288]
; CHECK: call i32 @__tgt_target_teams_mapper(ptr {{[^,]+}}, i64 {{[^,]+}}, ptr @[[KERNEL:[^,.]+]].region_id, {{.*}})
; CHECK: call void @[[KERNEL]](ptr %this1, i64 %x{{.*}})

; Test src:
;
; #include <stdio.h>
;
; class S {
; public:
;   int x;
;   int *y;
;   void foo() {
; #pragma omp target teams map(from: y[0:3]) num_teams(x)
;     {}
; //    printf("%p \n", y);
;   }
; };
;
; int main() {
;   int a[10];
;   S s;
;   s.x = 8;
;   s.y = &a[0];
;   a[2] = 111;
;   s.foo();
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

%class.S = type { i32, ptr }

$_ZN1S3fooEv = comdat any

define linkonce_odr dso_local void @_ZN1S3fooEv(ptr noundef nonnull align 8 dereferenceable(16) %this) comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  %x = getelementptr inbounds %class.S, ptr %this1, i32 0, i32 0
  %y = getelementptr inbounds %class.S, ptr %this1, i32 0, i32 1
  %y2 = getelementptr inbounds %class.S, ptr %this1, i32 0, i32 1
  %0 = load ptr, ptr %y2, align 8
  %arrayidx = getelementptr inbounds i32, ptr %0, i64 0
  %1 = getelementptr ptr, ptr %y, i32 1
  %2 = ptrtoint ptr %1 to i64
  %3 = ptrtoint ptr %y to i64
  %4 = sub i64 %2, %3
  %5 = sdiv exact i64 %4, ptrtoint (ptr getelementptr (i8, ptr null, i32 1) to i64)
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr %this1, ptr %y, i64 %5, i64 0, ptr null, ptr null), ; MAP type: 0 = 0x0
    "QUAL.OMP.MAP.FROM:CHAIN"(ptr %y, ptr %arrayidx, i64 12, i64 281474976710674, ptr null, ptr null), ; MAP type: 281474976710674 = 0x1000000000012 = MEMBER_OF_1 (0x1000000000000) | PTR_AND_OBJ (0x10) | FROM (0x2)
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %x, i32 0, i32 1) ]

  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.NUM_TEAMS:TYPED"(ptr %x, i32 0) ]

  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry() 

declare void @llvm.directive.region.exit(token) 

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 64773, i32 3825454, !"_ZN1S3fooEv", i32 8, i32 0, i32 0, i32 0}
