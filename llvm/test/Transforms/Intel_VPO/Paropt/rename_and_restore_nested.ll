; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -instcombine -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,instcombine,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR
;
; Test SRC:
;
;
; #include <stdio.h>
; typedef char TYPE;
;
; void work() {
; TYPE y[10];
;
; #pragma omp parallel private(y)
;   {
;     printf ("y = %d\n", y[0]);
; #pragma omp parallel private(y)
;     {
;       printf ("y[0] = %d\n", y[0]);
;     }
;   }
; }

; The test uses opaque pointers, but the bitcasts were retained from the typed-ptr version.

; Renaming of %y for outer region:
; PREPR: store ptr %y, ptr [[YADDR1:%[a-zA-Z._0-9]+]]
; PREPR: "QUAL.OMP.PRIVATE:TYPED"(ptr %y,
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(ptr %y, ptr [[YADDR1]])
; PREPR: [[YRENAMED1:%[a-zA-Z._0-9]+]] = load volatile ptr, ptr [[YADDR1]]
; PREPR: getelementptr inbounds [10 x i8], ptr [[YRENAMED1]]

; Renaming for inner region:
; PREPR: store ptr [[YRENAMED1]], ptr [[YADDR2:%[a-zA-Z._0-9]+]]
; PREPR: "QUAL.OMP.PRIVATE:TYPED"(ptr [[YRENAMED1]],
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(ptr [[YRENAMED1]], ptr [[YADDR2]])
; PREPR: [[YRENAMED2:%[a-zA-Z._0-9]+]] = load volatile ptr, ptr [[YADDR2]]
; PREPR: getelementptr inbounds [10 x i8], ptr [[YRENAMED2]]


; Check for the undone renaming after restore:
; RESTR-NOT alloca ptr
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"
; RESTR: "QUAL.OMP.PRIVATE:TYPED"(ptr %y,
; RESTR: getelementptr inbounds [10 x i8], ptr %y
; RESTR: "QUAL.OMP.PRIVATE:TYPED"(ptr %y,
; RESTR: getelementptr inbounds [10 x i8], ptr %y

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [8 x i8] c"y = %d\0A\00", align 1
@.str.1 = private unnamed_addr constant [11 x i8] c"y[0] = %d\0A\00", align 1

; Function Attrs: nounwind uwtable
define dso_local void @work() {
entry:
  %y = alloca [10 x i8], align 1
  %0 = bitcast ptr %y to ptr
  call void @llvm.lifetime.start.p0(i64 10, ptr %0)

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %y, i8 0, i64 10) ]

  %arrayidx = getelementptr inbounds [10 x i8], ptr %y, i64 0, i64 0
  %2 = load i8, ptr %arrayidx, align 1, !tbaa !2
  %conv = sext i8 %2 to i32
  %call = call i32 (ptr, ...) @printf(ptr @.str, i32 %conv)

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %y, i8 0, i64 10) ]

  %arrayidx1 = getelementptr inbounds [10 x i8], ptr %y, i64 0, i64 0
  %4 = load i8, ptr %arrayidx1, align 1, !tbaa !2
  %conv2 = sext i8 %4 to i32
  %call3 = call i32 (ptr, ...) @printf(ptr @.str.1, i32 %conv2)
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]



  %5 = bitcast ptr %y to ptr
  call void @llvm.lifetime.end.p0(i64 10, ptr %5)
  ret void
}

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(ptr, ...)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA10_c", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
