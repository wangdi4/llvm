; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -instcombine -S %s | FileCheck %s -check-prefix=INSTCMB
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,instcombine)' -S %s | FileCheck %s -check-prefix=INSTCMB
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -instcombine -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,instcombine,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR
;
; Test src:
;
;
; #include <string.h>
; #include <stdio.h>
;
; void print_int (void *c);
;
; int main() {
;
;   int y = 1;
;   print_int((void*) &y);
;
; #pragma omp parallel num_threads(1) firstprivate(y)
;   {
;     print_int((void*) &y);
;   }
;
;   return 0;
; }

; The test uses opaque pointers, but the bitcasts were retained from the typed-ptr version.

; Check for the renaming of %y after prepare vpo-paropt-prepare.
; PREPR: [[YADDR:%[a-zA-Z._0-9]+]] = alloca ptr
; PREPR: store ptr %y, ptr [[YADDR]]
; PREPR: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y,
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(ptr %y, ptr [[YADDR]])
; PREPR: [[YRENAMED:%[a-zA-Z._0-9]+]] = load volatile ptr, ptr [[YADDR]]
; PREPR: [[YRENAMED_BC:%[a-zA-Z._0-9]+]] = bitcast ptr [[YRENAMED]] to ptr
; PREPR: call void @print_int(ptr [[YRENAMED_BC]])

; Check that the IR was not modified by CSE + Instcombine

; INSTCMB: [[YADDR:%[a-zA-Z._0-9]+]] = alloca ptr
; INSTCMB: "QUAL.OMP.OPERAND.ADDR"(ptr %y, ptr [[YADDR]])
; INSTCMB: [[YRENAMED:%[a-zA-Z._0-9]+]] = load volatile ptr, ptr [[YADDR]]
; INSTCMB: call void @print_int(ptr [[YRENAMED]])

; Check for restore-operands was able to undo the renaming:
; RESTR: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y,
; RESTR-NOT: alloca ptr
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"
; RESTR: call void @print_int(ptr %y)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %y = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  %0 = bitcast ptr %y to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %0)
  store i32 1, ptr %y, align 4, !tbaa !4
  %1 = bitcast ptr %y to ptr
  call void @print_int(ptr %1)
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.NUM_THREADS"(i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y, i32 0, i32 1) ]

  %3 = bitcast ptr %y to ptr
  call void @print_int(ptr %3)
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]

  %4 = bitcast ptr %y to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr %4)
  ret i32 0
}

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)
declare dso_local void @print_int(ptr)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{!"clang version 17.0.0"}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
