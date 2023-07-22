; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -instcombine -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,instcombine,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR
;
; Test src:
;
; #include <stdio.h>
; typedef char TYPE;
;
; void work() {
; TYPE y[10];
;
; #pragma omp parallel firstprivate(y)
;   printf ("y[1] = %d\n", y[1]);
;
; #pragma omp parallel firstprivate(y)
;   printf ("y[1] = %d\n", y[1]);
; }

; The test uses opaque pointers, but the bitcasts were retained from the typed-ptr version.

; Renaming of %y for first region:
; PREPR: store ptr %y, ptr [[YADDR1:%[a-zA-Z._0-9]+]]
; PREPR: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y,
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(ptr %y, ptr [[YADDR1]])
; PREPR: [[YRENAMED1:%[a-zA-Z._0-9]+]] = load volatile ptr, ptr [[YADDR1]]
; PREPR: getelementptr inbounds [10 x i8], ptr [[YRENAMED1]]
; PREPR: "DIR.OMP.END.PARALLEL"

; Renaming for second region:
; PREPR: store ptr %y, ptr [[YADDR2:%[a-zA-Z._0-9]+]]
; PREPR: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y,
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(ptr %y, ptr [[YADDR2]])
; PREPR: [[YRENAMED2:%[a-zA-Z._0-9]+]] = load volatile ptr, ptr [[YADDR2]]
; PREPR: getelementptr inbounds [10 x i8], ptr [[YRENAMED2]]
; PREPR: "DIR.OMP.END.PARALLEL"

; Check for the undone renaming after restore:
; RESTR-NOT alloca [10 x i8]*
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"
; RESTR: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y,
; RESTR: getelementptr inbounds [10 x i8], ptr %y
; RESTR: "DIR.OMP.END.PARALLEL"
; RESTR: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y,
; RESTR: getelementptr inbounds [10 x i8], ptr %y
; RESTR: "DIR.OMP.END.PARALLEL"

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [11 x i8] c"y[1] = %d\0A\00", align 1

; Function Attrs: nounwind uwtable
define dso_local void @work() {
entry:
  %y = alloca [10 x i8], align 1
  %0 = bitcast ptr %y to ptr
  call void @llvm.lifetime.start.p0(i64 10, ptr %0)
  %array.begin = getelementptr inbounds [10 x i8], ptr %y, i32 0, i32 0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y, i8 0, i64 10) ]

  %arrayidx = getelementptr inbounds [10 x i8], ptr %y, i64 0, i64 1
  %2 = load i8, ptr %arrayidx, align 1, !tbaa !4
  %conv = sext i8 %2 to i32
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %conv)
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]

  %array.begin1 = getelementptr inbounds [10 x i8], ptr %y, i32 0, i32 0
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y, i8 0, i64 10) ]

  %arrayidx2 = getelementptr inbounds [10 x i8], ptr %y, i64 0, i64 1
  %4 = load i8, ptr %arrayidx2, align 1, !tbaa !4
  %conv3 = sext i8 %4 to i32
  %call4 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %conv3)
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL"() ]

  %5 = bitcast ptr %y to ptr
  call void @llvm.lifetime.end.p0(i64 10, ptr %5)
  ret void
}

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(ptr noundef, ...)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)


!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{!"clang version 17.0.0"}
!4 = !{!5, !6, i64 0}
!5 = !{!"array@_ZTSA10_c", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
