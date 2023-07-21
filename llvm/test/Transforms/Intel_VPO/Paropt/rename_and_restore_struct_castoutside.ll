; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -instcombine -S %s | FileCheck %s -check-prefix=INSTCMB
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,instcombine)' -S %s | FileCheck %s -check-prefix=INSTCMB
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -instcombine -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,instcombine,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR

; Test src:
;
; #include <stdio.h>
;
; typedef struct _FourChars { char a, b, c, d; } FourChars;
;
; __attribute__((noinline)) void foo(const FourChars f) {
;   #pragma omp parallel shared(f) num_threads(1)
;   {
;     printf("a = %c\n", f.a);
;   }
; }

; The test uses opaque pointers, but the bitcasts were retained from the typed-ptr version.

; Check for the renaming of %y after prepare vpo-paropt-prepare.
; PREPR: [[FADDR:%[a-zA-Z._0-9]+]] = alloca ptr
; PREPR: store ptr %f, ptr [[FADDR]]
; PREPR: "QUAL.OMP.SHARED:TYPED"(ptr %f,
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(ptr %f, ptr [[FADDR]])
; PREPR: [[FRENAMED:%[a-zA-Z._0-9]+]] = load volatile ptr, ptr [[FADDR]]
; PREPR: getelementptr inbounds %struct._FourChars, ptr [[FRENAMED]]

; Check that the IR was not modified by CSE + Instcombine

; INSTCMB: [[F:%[a-zA-Z._0-9]+]] = alloca %struct._FourChars
; INSTCMB: [[FADDR:%[a-zA-Z._0-9]+]] = alloca ptr
; INSTCMB: store ptr [[F]], ptr [[FADDR]]
; INSTCMB: "QUAL.OMP.OPERAND.ADDR"(ptr [[F]], ptr [[FADDR]])
; INSTCMB: [[FRENAMED:%[a-zA-Z._0-9]+]] = load volatile ptr, ptr [[FADDR]]
; INSTCMB: getelementptr inbounds %struct._FourChars, ptr [[FRENAMED]]

; Check for restore-operands was able to undo the renaming:
; RESTR: [[F:%[a-zA-Z._0-9]+]] = alloca %struct._FourChars
; RESTR: "QUAL.OMP.SHARED:TYPED"(ptr [[F]],
; RESTR-NOT: alloca ptr
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"
; RESTR: getelementptr inbounds %struct._FourChars, ptr [[F]]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._FourChars = type { i8, i8, i8, i8 }

@.str = private unnamed_addr constant [8 x i8] c"a = %c\0A\00", align 1

define dso_local void @foo(i32 %f.coerce) {
entry:
  %f = alloca %struct._FourChars, align 1
  %0 = bitcast ptr %f to ptr
  store i32 %f.coerce, ptr %0, align 1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %f, %struct._FourChars zeroinitializer, i32 1),
    "QUAL.OMP.NUM_THREADS"(i32 1) ]

  %a = getelementptr inbounds %struct._FourChars, ptr %f, i32 0, i32 0
  %2 = load i8, ptr %a, align 1
  %conv = sext i8 %2 to i32
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %conv)
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(ptr noundef, ...)
