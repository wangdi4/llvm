; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -instcombine -early-cse -S %s | FileCheck %s -check-prefix=INSTCMB
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,instcombine,early-cse)' -S %s | FileCheck %s -check-prefix=INSTCMB
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -instcombine -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,instcombine,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR
;
; Test Src:
;
; #include <stdio.h>
;
; typedef struct _FourChars { char a, b, c, d; } FourChars;
; void print_str(int* s);
;
; __attribute__((noinline)) void foo(const FourChars f) {
;   #pragma omp parallel firstprivate(f) num_threads(1)
;   {
;     print_str((int*) &f);
;   }
; }

; The test uses opaque pointers, but the bitcasts were retained from the typed-ptr version.

; Check for the renaming of %y after prepare vpo-paropt-prepare.
; PREPR: [[FADDR:%[a-zA-Z._0-9]+]] = alloca ptr
; PREPR: store ptr %f, ptr [[FADDR]]
; PREPR: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %f,
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(ptr %f, ptr [[FADDR]])
; PREPR: [[FRENAMED:%[a-zA-Z._0-9]+]] = load volatile ptr, ptr [[FADDR]]
; PREPR: [[FRENAMED_BC:%[a-zA-Z._0-9]+]] = bitcast ptr [[FRENAMED]] to ptr
; PREPR: call void @print_str(ptr [[FRENAMED_BC]])

; Check that the IR was not modified by CSE + Instcombine

; INSTCMB: [[F:%[a-zA-Z._0-9]+]] = alloca %struct._FourChars
; INSTCMB: [[FADDR:%[a-zA-Z._0-9]+]] = alloca ptr
; INSTCMB: store ptr [[F]], ptr [[FADDR]]
; INSTCMB: "QUAL.OMP.OPERAND.ADDR"(ptr [[F]], ptr [[FADDR]])
; INSTCMB: [[FRENAMED:%[a-zA-Z._0-9]+]] = load volatile ptr, ptr [[FADDR]]
; INSTCMB: call void @print_str(ptr [[FRENAMED]])

; Check for restore-operands was able to undo the renaming:
; RESTR: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %f,
; RESTR-NOT: alloca ptr
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"
; RESTR: call void @print_str(ptr %f)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._FourChars = type { i8, i8, i8, i8 }

define dso_local void @foo(i32 %f.coerce) {
entry:
  %f = alloca %struct._FourChars, align 1
  %0 = bitcast ptr %f to ptr
  store i32 %f.coerce, ptr %0, align 1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %f, %struct._FourChars zeroinitializer, i32 1),
    "QUAL.OMP.NUM_THREADS"(i32 1) ]

  %2 = bitcast ptr %f to ptr
  call void @print_str(ptr %2)
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local void @print_str(ptr)
