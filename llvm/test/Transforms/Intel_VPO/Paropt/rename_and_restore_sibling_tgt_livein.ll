; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR

; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -instcombine -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,instcombine,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR

; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -early-cse -instcombine -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefix=TFORM
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,early-cse,instcombine,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefix=TFORM

; test src:

; #include <stdio.h>
; typedef short TYPE;
; #pragma omp begin declare target
; TYPE y[10];
; #pragma omp end declare target
;
; void work() {
;
; #pragma omp target
;   printf ("&y[1] = %p\n", &y[1]);
;
; #pragma omp target
;   printf ("&y[1] = %p\n", &y[1]);
; }

; The test was hand-modified to change the MAP clause on y to LIVEIN.

; Check that after -vpo-paropt-prepare, the OPERAND.ADDR renaming code is
; generated for LIVEIN operands
; PREPR-COUNT-2: call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),{{.*}}"QUAL.OMP.LIVEIN"([10 x i16]* @y), "QUAL.OMP.OPERAND.ADDR"([10 x i16]* @y, [10 x i16]** %y.addr{{.*}}){{.*}} ]

; Check that after -vpo-restore-operands, the OERAND.ADDR renaming has been
; undone.
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"
; RESTR-NOT: %y.addr
; RESTR-COUNT-2: call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),{{.*}}"QUAL.OMP.LIVEIN"([10 x i16]* @y){{.*}} ]

; Check that the outlined functions created for the target constructs
; don't pass y as an argument.
; TFORM: call void @__omp_offloading_10309_7086c47__Z4work_l9()
; TFORM: call void @__omp_offloading_10309_7086c47__Z4work_l12()

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64"
target device_triples = "x86_64"

@y = protected target_declare global [10 x i16] zeroinitializer, align 16
@.str = private unnamed_addr constant [12 x i8] c"&y[1] = %p\0A\00", align 1

; Function Attrs: convergent noinline nounwind optnone uwtable
define protected void @work() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1), "QUAL.OMP.LIVEIN"([10 x i16]* @y) ]
  %call = call i32 (i8*, ...) @printf(i8* noundef getelementptr inbounds ([12 x i8], [12 x i8]* @.str, i64 0, i64 0), i16* noundef getelementptr inbounds ([10 x i16], [10 x i16]* @y, i64 0, i64 1)) #3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 2), "QUAL.OMP.LIVEIN"([10 x i16]* @y) ]
  %call1 = call i32 (i8*, ...) @printf(i8* noundef getelementptr inbounds ([12 x i8], [12 x i8]* @.str, i64 0, i64 0), i16* noundef getelementptr inbounds ([10 x i16], [10 x i16]* @y, i64 0, i64 1)) #3
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: convergent
declare i32 @printf(i8* noundef, ...) #2

attributes #0 = { convergent noinline nounwind optnone uwtable "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { convergent "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { convergent nounwind }

!omp_offload.info = !{!0, !1, !2}
!llvm.module.flags = !{!3, !4, !5, !6, !7, !8}

!0 = !{i32 0, i32 66313, i32 117992519, !"_Z4work", i32 9, i32 1, i32 0}
!1 = !{i32 0, i32 66313, i32 117992519, !"_Z4work", i32 12, i32 2, i32 0}
!2 = !{i32 1, !"_Z1y", i32 0, i32 0, [10 x i16]* @y}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 7, !"openmp", i32 50}
!5 = !{i32 7, !"openmp-device", i32 50}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 7, !"frame-pointer", i32 2}
