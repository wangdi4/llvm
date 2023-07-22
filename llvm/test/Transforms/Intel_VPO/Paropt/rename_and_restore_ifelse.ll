; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -instcombine -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,instcombine,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR

; Test src:
;
; #include <stdio.h>
; typedef char TYPE;
;
; extern volatile bool flag;
;
; void work() {
;   TYPE *y;
;
;   if (flag)
; #pragma omp parallel firstprivate(y)
;     printf ("y = %p\n", y);
;   else
; #pragma omp parallel firstprivate(y)
;     printf ("y = %p\n", y);
; }

; The test uses opaque pointers, but the bitcasts were retained from the typed-ptr version.

; Renaming of %y for first region:
; PREPR: store ptr %y, ptr [[YADDR1:%[a-zA-Z._0-9]+]]
; PREPR: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y,
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(ptr %y, ptr [[YADDR1]])
; PREPR: [[YRENAMED1:%[a-zA-Z._0-9]+]] = load volatile ptr, ptr [[YADDR1]]
; PREPR: load ptr, ptr [[YRENAMED1]]
; PREPR: "DIR.OMP.END.PARALLEL"

; Renaming for inner region:
; PREPR: store ptr %y, ptr [[YADDR2:%[a-zA-Z._0-9]+]]
; PREPR: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y,
; PREPR-SAME: "QUAL.OMP.OPERAND.ADDR"(ptr %y, ptr [[YADDR2]])
; PREPR: [[YRENAMED2:%[a-zA-Z._0-9]+]] = load volatile ptr, ptr [[YADDR2]]
; PREPR: load ptr, ptr [[YRENAMED2]]
; PREPR: "DIR.OMP.END.PARALLEL"

; Check for the undone renaming after restore:
; RESTR-NOT alloca ptr
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"
; RESTR: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y,
; RESTR: load ptr, ptr %y
; RESTR: "DIR.OMP.END.PARALLEL"
; RESTR: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y,
; RESTR: load ptr, ptr %y
; RESTR: "DIR.OMP.END.PARALLEL"


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@flag = external dso_local global i8, align 1
@.str = private unnamed_addr constant [8 x i8] c"y = %p\0A\00", align 1

; Function Attrs: nounwind uwtable
define dso_local void @work() {
entry:
  %y = alloca ptr, align 8
  %0 = bitcast ptr %y to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr %0)
  %1 = load volatile i8, ptr @flag, align 1, !tbaa !4
  %tobool = icmp ne i8 %1, 0
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y, ptr null, i32 1) ]

  %3 = load ptr, ptr %y, align 8, !tbaa !7
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, ptr noundef %3)
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]

  br label %if.end

if.else:                                          ; preds = %entry
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %y, ptr null, i32 1) ]

  %5 = load ptr, ptr %y, align 8, !tbaa !7
  %call1 = call i32 (ptr, ...) @printf(ptr noundef @.str, ptr noundef %5)
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL"() ]

  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %6 = bitcast ptr %y to ptr
  call void @llvm.lifetime.end.p0(i64 8, ptr %6)
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
!4 = !{!5, !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"pointer@_ZTSPc", !5, i64 0}
