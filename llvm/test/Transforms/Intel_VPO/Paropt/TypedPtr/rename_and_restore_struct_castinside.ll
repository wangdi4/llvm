; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -instcombine -S %s | FileCheck %s -check-prefix=INSTCMB
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,instcombine)' -S %s | FileCheck %s -check-prefix=INSTCMB
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -instcombine -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,instcombine,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR

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

; Check for the renaming of %y after prepare vpo-paropt-prepare.
; PREPR: [[YADDR:%[a-zA-Z._0-9]+]] = alloca %struct.str*
; PREPR: store %struct.str* %y, %struct.str** [[YADDR]]
; PREPR: "QUAL.OMP.FIRSTPRIVATE:TYPED"(%struct.str* %y,
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(%struct.str* %y, %struct.str** [[YADDR]])
; PREPR: [[YRENAMED:%[a-zA-Z._0-9]+]] = load volatile %struct.str*, %struct.str** [[YADDR]]
; PREPR: [[YRENAMED_BC:%[a-zA-Z._0-9]+]] = bitcast %struct.str* [[YRENAMED]] to i8*
; PREPR: call void @print_str(i8* [[YRENAMED_BC]])

; Check that the IR was not modified by CSE + Instcombine

; INSTCMB: [[YADDR:%[a-zA-Z._0-9]+]] = alloca %struct.str*
; INSTCMB: store %struct.str* %y, %struct.str** [[YADDR]]
; INSTCMB: "QUAL.OMP.FIRSTPRIVATE:TYPED"(%struct.str* %y,
; INSTCMB-SAME: "QUAL.OMP.OPERAND.ADDR"(%struct.str* %y, %struct.str** [[YADDR]])
; INSTCMB: [[YRENAMED:%[a-zA-Z._0-9]+]] = load volatile %struct.str*, %struct.str** [[YADDR]]
; INSTCMB: [[YRENAMED_BC:%[a-zA-Z._0-9]+]] = bitcast %struct.str* [[YRENAMED]] to i8*
; INSTCMB: call void @print_str(i8* [[YRENAMED_BC]])

; Check for restore-operands was able to undo the renaming:
; RESTR: "QUAL.OMP.FIRSTPRIVATE:TYPED"(%struct.str* %y,
; RESTR-NOT: alloca %struct.str*
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"
; RESTR: [[YRENAMED:%[a-zA-Z._0-9]+]] = bitcast %struct.str* %y to i8*
; RESTR: call void @print_str(i8* [[YRENAMED]])

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.str = type { i32, i8 }

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %y = alloca %struct.str, align 4
  store i32 0, i32* %retval, align 4
  %0 = bitcast %struct.str* %y to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %0)
  %a = getelementptr inbounds %struct.str, %struct.str* %y, i32 0, i32 0
  store i32 1, i32* %a, align 4
  %b = getelementptr inbounds %struct.str, %struct.str* %y, i32 0, i32 1
  store i8 97, i8* %b, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.NUM_THREADS"(i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(%struct.str* %y, %struct.str zeroinitializer, i32 1) ]

  %2 = bitcast %struct.str* %y to i8*
  call void @print_str(i8* %2)
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]

  %3 = bitcast %struct.str* %y to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %3)
  ret i32 0
}

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local void @print_str(i8* noundef)
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)
