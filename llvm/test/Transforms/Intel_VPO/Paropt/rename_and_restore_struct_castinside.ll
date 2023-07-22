; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -instcombine -S %s | FileCheck %s -check-prefix=INSTCMB
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,instcombine)' -S %s | FileCheck %s -check-prefix=INSTCMB
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -instcombine -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,instcombine,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR

; Test src:
;
; #include <string.h>
; #include <stdio.h>
;
; typedef struct {int a; char b; } str;
;
; void print_str(void *s);
;
; int main() {
;   str y;
;   y.a = 1; y.b = 'a';
;
; #pragma omp parallel num_threads(1) firstprivate(y)
;   {
;     print_str((void*) &y);
;   }
;   return 0;
; }
;

; The test uses opaque pointers, but the bitcasts were retained from the typed-ptr version.

; Check for the renaming of %y after prepare vpo-paropt-prepare.
; PREPR: [[YADDR:%[a-zA-Z._0-9]+]] = alloca ptr
; PREPR: store ptr %y, ptr [[YADDR]]
; PREPR: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y,
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(ptr %y, ptr [[YADDR]])
; PREPR: [[YRENAMED:%[a-zA-Z._0-9]+]] = load volatile ptr, ptr [[YADDR]]
; PREPR: [[YRENAMED_BC:%[a-zA-Z._0-9]+]] = bitcast ptr [[YRENAMED]] to ptr
; PREPR: call void @print_str(ptr [[YRENAMED_BC]])

; Check that the IR was not modified by CSE + Instcombine

; INSTCMB: [[YADDR:%[a-zA-Z._0-9]+]] = alloca ptr
; INSTCMB: store ptr %y, ptr [[YADDR]]
; INSTCMB: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y,
; INSTCMB-SAME: "QUAL.OMP.OPERAND.ADDR"(ptr %y, ptr [[YADDR]])
; INSTCMB: [[YRENAMED:%[a-zA-Z._0-9]+]] = load volatile ptr, ptr [[YADDR]]
; INSTCMB: call void @print_str(ptr [[YRENAMED]])

; Check for restore-operands was able to undo the renaming:
; RESTR: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y,
; RESTR-NOT: alloca ptr
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"
; RESTR: call void @print_str(ptr %y)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.str = type { i32, i8 }

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %y = alloca %struct.str, align 4
  store i32 0, ptr %retval, align 4
  %0 = bitcast ptr %y to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr %0)
  %a = getelementptr inbounds %struct.str, ptr %y, i32 0, i32 0
  store i32 1, ptr %a, align 4
  %b = getelementptr inbounds %struct.str, ptr %y, i32 0, i32 1
  store i8 97, ptr %b, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.NUM_THREADS"(i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y, %struct.str zeroinitializer, i32 1) ]

  %2 = bitcast ptr %y to ptr
  call void @print_str(ptr %2)
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]

  %3 = bitcast ptr %y to ptr
  call void @llvm.lifetime.end.p0(i64 8, ptr %3)
  ret i32 0
}

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local void @print_str(ptr noundef)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)
