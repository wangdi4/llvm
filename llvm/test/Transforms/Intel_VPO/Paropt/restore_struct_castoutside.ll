; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR

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

; This test is a subset of rename_and_restore_struct_castoutside.ll, which captures
; the IR after instcombine, and does not depend on the optimization's behavior to
; stay the same (for example, %f1 is not allocated as an i32 by instcombine
; with opaque-pointers).

; The test uses opaque pointers, but the bitcasts were retained from the typed-ptr version.

; Check for restore-operands was able to undo the renaming:
; RESTR: [[F:%[a-zA-Z._0-9]+]] = alloca i32
; RESTR: [[FCAST:%[a-zA-Z._0-9]+]] = bitcast ptr [[F]] to ptr
; RESTR: "QUAL.OMP.SHARED:TYPED"(ptr [[FCAST]],
; RESTR-NOT: alloca ptr
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"
; RESTR: getelementptr inbounds %struct._FourChars, ptr [[FCAST]]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._FourChars = type { i8, i8, i8, i8 }

@.str = private unnamed_addr constant [8 x i8] c"a = %c\0A\00", align 1
@"@tid.addr" = external global i32

define dso_local void @foo(i32 %f.coerce) {
entry:
  %f = alloca i32, align 4
  %tmpcast = bitcast ptr %f to ptr
  store i32 %f.coerce, ptr %f, align 4
  %f.addr = alloca ptr, align 8
  %0 = bitcast ptr %f.addr to ptr
  store ptr %f, ptr %0, align 8
  %end.dir.temp = alloca i1, align 1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %tmpcast, %struct._FourChars zeroinitializer, i32 1),
    "QUAL.OMP.NUM_THREADS"(i32 1),
    "QUAL.OMP.OPERAND.ADDR"(ptr %tmpcast, ptr %f.addr),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]

  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.PARALLEL.4.split, label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %entry
  %f1 = load volatile ptr, ptr %f.addr, align 8
  %a = getelementptr inbounds %struct._FourChars, ptr %f1, i64 0, i32 0
  %2 = load i8, ptr %a, align 1
  %conv = sext i8 %2 to i32
  %call = call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str, i32 noundef %conv)
  br label %DIR.OMP.END.PARALLEL.4.split

DIR.OMP.END.PARALLEL.4.split:                     ; preds = %entry, %DIR.OMP.PARALLEL.3
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(ptr noundef, ...)
