; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -instcombine -S %s | FileCheck %s -check-prefix=INSTCMB
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,instcombine)' -S %s | FileCheck %s -check-prefix=INSTCMB
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -instcombine -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,instcombine,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR

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

; Check for the renaming of %y after prepare vpo-paropt-prepare.
; PREPR: [[FADDR:%[a-zA-Z._0-9]+]] = alloca %struct._FourChars*
; PREPR: store %struct._FourChars* %f, %struct._FourChars** [[FADDR]]
; PREPR: "QUAL.OMP.SHARED:TYPED"(%struct._FourChars* %f,
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(%struct._FourChars* %f, %struct._FourChars** [[FADDR]])
; PREPR: [[FRENAMED:%[a-zA-Z._0-9]+]] = load volatile %struct._FourChars*, %struct._FourChars** [[FADDR]]
; PREPR: getelementptr inbounds %struct._FourChars, %struct._FourChars* [[FRENAMED]]

; Check that the IR was not modified by CSE + Instcombine

; INSTCMB: [[F:%[a-zA-Z._0-9]+]] = alloca i32
; INSTCMB: [[FCAST:%[a-zA-Z._0-9]+]] = bitcast i32* [[F]] to %struct._FourChars*
; INSTCMB: [[FADDR:%[a-zA-Z._0-9]+]] = alloca %struct._FourChars*
; INSTCMB: [[FADDR_CAST:%[a-zA-Z._0-9]+]] = bitcast %struct._FourChars** [[FADDR]] to i32**
; INSTCMB: store i32* [[F]], i32** [[FADDR_CAST]]
; INSTCMB: "QUAL.OMP.OPERAND.ADDR"(%struct._FourChars* [[FCAST]], %struct._FourChars** [[FADDR]])
; INSTCMB: [[FRENAMED:%[a-zA-Z._0-9]+]] = load volatile %struct._FourChars*, %struct._FourChars** [[FADDR]]
; INSTCMB: getelementptr inbounds %struct._FourChars, %struct._FourChars* [[FRENAMED]]

; Check for restore-operands was able to undo the renaming:
; RESTR: [[F:%[a-zA-Z._0-9]+]] = alloca i32
; RESTR: [[FCAST:%[a-zA-Z._0-9]+]] = bitcast i32* [[F]] to %struct._FourChars*
; RESTR: "QUAL.OMP.SHARED:TYPED"(%struct._FourChars* [[FCAST]],
; RESTR-NOT: alloca %struct._FourChars*
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"
; RESTR: getelementptr inbounds %struct._FourChars, %struct._FourChars* [[FCAST]]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._FourChars = type { i8, i8, i8, i8 }

@.str = private unnamed_addr constant [8 x i8] c"a = %c\0A\00", align 1

define dso_local void @foo(i32 %f.coerce) {
entry:
  %f = alloca %struct._FourChars, align 1
  %0 = bitcast %struct._FourChars* %f to i32*
  store i32 %f.coerce, i32* %0, align 1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(%struct._FourChars* %f, %struct._FourChars zeroinitializer, i32 1),
    "QUAL.OMP.NUM_THREADS"(i32 1) ]

  %a = getelementptr inbounds %struct._FourChars, %struct._FourChars* %f, i32 0, i32 0
  %2 = load i8, i8* %a, align 1
  %conv = sext i8 %2 to i32
  %call = call i32 (i8*, ...) @printf(i8* noundef getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0), i32 noundef %conv)
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(i8* noundef, ...)
