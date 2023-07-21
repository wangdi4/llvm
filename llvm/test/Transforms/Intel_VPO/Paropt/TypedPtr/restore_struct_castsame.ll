; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR

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

; This test is a subset of rename_and_restore_struct_castsame.ll, which captures
; the IR after instcombine, and does not depend on the optimization's behavior to
; stay the same (for example, %f1 is not allocated as an i32 by instcombine
; with opaque-pointers).

; Check for restore-operands was able to undo the renaming:
; RESTR: [[F:%[a-zA-Z._0-9]+]] = alloca i32
; RESTR: [[FCAST:%[a-zA-Z._0-9]+]] = bitcast i32* [[F]] to %struct._FourChars*
; RESTR: "QUAL.OMP.FIRSTPRIVATE:TYPED"(%struct._FourChars* [[FCAST]],
; RESTR-NOT: alloca %struct._FourChars*
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"
; RESTR: [[FRENAMED:%[a-zA-Z._0-9]+]] = bitcast %struct._FourChars* [[FCAST]] to i32*
; RESTR: call void @print_str(i32* [[FRENAMED]])

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._FourChars = type { i8, i8, i8, i8 }

@"@tid.addr" = external global i32

define dso_local void @foo(i32 %f.coerce) {
entry:
  %f = alloca i32, align 4
  %tmpcast = bitcast i32* %f to %struct._FourChars*
  store i32 %f.coerce, i32* %f, align 4
  %f.addr = alloca %struct._FourChars*, align 8
  %0 = bitcast %struct._FourChars** %f.addr to i32**
  store i32* %f, i32** %0, align 8
  %end.dir.temp = alloca i1, align 1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(%struct._FourChars* %tmpcast, %struct._FourChars zeroinitializer, i32 1),
    "QUAL.OMP.NUM_THREADS"(i32 1),
    "QUAL.OMP.OPERAND.ADDR"(%struct._FourChars* %tmpcast, %struct._FourChars** %f.addr),
    "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]

  %temp.load = load volatile i1, i1* %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.PARALLEL.4.split, label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %entry
  %f1 = load volatile %struct._FourChars*, %struct._FourChars** %f.addr, align 8
  %2 = bitcast %struct._FourChars* %f1 to i32*
  call void @print_str(i32* %2)
  br label %DIR.OMP.END.PARALLEL.4.split

DIR.OMP.END.PARALLEL.4.split:                     ; preds = %entry, %DIR.OMP.PARALLEL.3
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local void @print_str(i32*)
