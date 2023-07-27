; INTEL_CUSTOMIZATION
; RUN: opt --passes="vpo-paropt-loop-collapse" -S %s | FileCheck %s

; More complex version of collapse-sect.ll.
; PARALLEL LOOP {
;   SECTION
;   END SECTION
;   SECTION
;     CODE
;   END SECTION
;     ...
; } END PARALLEL LOOP
;
; This had two errors:
; After the sections were lowered to an inner loop+switch, the exit block of
; the section loop was not added to the outer parallel loop.
;
; The "CODE" block(s) were not added to the inner loop.

; CHECK-LABEL: alloca_0
; CHECK-LABEL: bb13
; CHECK-LABEL: bb32
; CHECK-LABEL: call.post.list361
; CHECK-LABEL: ccsd_trpdrv_omp_.sw.case1.1
; CHECK-LABEL: call.post.list373
; CHECK-LABEL: ccsd_trpdrv_omp_.sw.case2.1
; CHECK-LABEL: call.post.list385
; CHECK-LABEL: ccsd_trpdrv_omp_.sw.case3.1
; CHECK-LABEL: call.post.list397
; CHECK-LABEL: ccsd_trpdrv_omp_.sw.case4.1
; CHECK-LABEL: call.post.list409
; CHECK-LABEL: ccsd_trpdrv_omp_.sw.case5.1
; CHECK-LABEL: DIR.OMP.SECTION.10
; CHECK-LABEL: call.post.list421
; CHECK-LABEL: ccsd_trpdrv_omp_.sw.case6.1
; CHECK-LABEL: call.post.list433
; CHECK-LABEL: ccsd_trpdrv_omp_.sw.case7.1
; CHECK-LABEL: call.post.list445
; CHECK-LABEL: DIR.OMP.END.SECTION.15
; CHECK-LABEL: DIR.OMP.END.SECTIONS.16
; CHECK-LABEL: DIR.OMP.MASKED.17
; CHECK-LABEL: call.pre.list450_then
; CHECK-LABEL: call.pre.list462_else
; CHECK-LABEL: bb43_endif
; CHECK-LABEL: DIR.OMP.END.MASKED.18
; CHECK-LABEL: omp.pdo.cond476
; CHECK-LABEL: omp.pdo.body477
; CHECK-LABEL: do.cond482
; CHECK-LABEL: do.body483
; CHECK-LABEL: do.epilog484
; CHECK-LABEL: bb_new479
; CHECK-LABEL: bb_new479.split
; CHECK-LABEL: DIR.OMP.LOOP.20
; CHECK-LABEL: DIR.OMP.LOOP.20.split
; CHECK-LABEL: omp.collapsed.loop.cond
; CHECK-LABEL: omp.collapsed.loop.body
; CHECK-LABEL: omp.collapsed.loop.exit
; CHECK-LABEL: omp.collapsed.loop.inc
; CHECK-LABEL: omp.collapsed.loop.postexit
; CHECK-LABEL: DIR.OMP.END.LOOP.21
; CHECK-LABEL: DIR.OMP.PARALLEL.23
; CHECK-LABEL: bb_new352
; CHECK-LABEL: .sloop.header.1
; CHECK-LABEL: .sloop.body.1
; CHECK-LABEL: ccsd_trpdrv_omp_.sw.succBB.1
; CHECK-LABEL: ccsd_trpdrv_omp_.sloop.latch.1
; CHECK-LABEL: ccsd_trpdrv_omp_.sw.case0.1
; CHECK-LABEL: .sloop.preheader.1
; CHECK-LABEL: ccsd_trpdrv_omp_.sw.epilog.1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@len_ = external unnamed_addr global [56 x i8], align 32
@cccsds_ = external unnamed_addr global [240 x i8], align 32

define void @ccsd_trpdrv_omp_() {
alloca_0:
  br label %bb13

bb13:                                             ; preds = %alloca_0
  br label %bb32

bb32:                                             ; preds = %bb13, %DIR.OMP.END.LOOP.21:
  br label %DIR.OMP.PARALLEL.23

call.post.list361:                                ; preds = %DIR.OMP.SECTIONS.24
  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.SECTION"() ]
  br label %DIR.OMP.END.SECTION.1

DIR.OMP.END.SECTION.1:                            ; preds = %call.post.list361
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  br label %call.post.list373

call.post.list373:                                ; preds = %DIR.OMP.END.SECTION.1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SECTION"() ]
  br label %DIR.OMP.END.SECTION.3

DIR.OMP.END.SECTION.3:                            ; preds = %call.post.list373
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  br label %call.post.list385

call.post.list385:                                ; preds = %DIR.OMP.END.SECTION.3
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SECTION"() ]
  br label %DIR.OMP.END.SECTION.5

DIR.OMP.END.SECTION.5:                            ; preds = %call.post.list385
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  br label %call.post.list397

call.post.list397:                                ; preds = %DIR.OMP.END.SECTION.5
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SECTION"() ]
  br label %DIR.OMP.END.SECTION.7

DIR.OMP.END.SECTION.7:                            ; preds = %call.post.list397
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  br label %call.post.list409

call.post.list409:                                ; preds = %DIR.OMP.END.SECTION.7
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SECTION"() ]
  br label %DIR.OMP.END.SECTION.9

DIR.OMP.END.SECTION.9:                            ; preds = %call.post.list409
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  br label %DIR.OMP.SECTION.10

DIR.OMP.SECTION.10:                               ; preds = %DIR.OMP.END.SECTION.9
  br label %call.post.list421

call.post.list421:                                ; preds = %DIR.OMP.SECTION.10
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.SECTION"() ]
  br label %DIR.OMP.END.SECTION.11

DIR.OMP.END.SECTION.11:                           ; preds = %call.post.list421
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  br label %call.post.list433

call.post.list433:                                ; preds = %DIR.OMP.END.SECTION.11
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.SECTION"() ]
  br label %DIR.OMP.END.SECTION.13

DIR.OMP.END.SECTION.13:                           ; preds = %call.post.list433
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  br label %call.post.list445

call.post.list445:                                ; preds = %DIR.OMP.END.SECTION.13
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.SECTION"() ]
  br label %DIR.OMP.END.SECTION.15

DIR.OMP.END.SECTION.15:                           ; preds = %call.post.list445
  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.SECTIONS"() ]
  br label %DIR.OMP.END.SECTIONS.16

DIR.OMP.END.SECTIONS.16:                          ; preds = %DIR.OMP.END.SECTION.15
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASKED"() ]
  br label %DIR.OMP.MASKED.17

DIR.OMP.MASKED.17:                                ; preds = %DIR.OMP.END.SECTIONS.16
  br i1 undef, label %call.pre.list450_then, label %call.pre.list462_else

call.pre.list450_then:                            ; preds = %DIR.OMP.MASKED.17
  br label %bb43_endif

call.pre.list462_else:                            ; preds = %DIR.OMP.MASKED.17
  unreachable

bb43_endif:                                       ; preds = %call.pre.list450_then
  br label %DIR.OMP.END.MASKED.18

DIR.OMP.END.MASKED.18:                            ; preds = %bb43_endif
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.MASKED"() ]
  br label %bb_new479

omp.pdo.cond476:                                  ; preds = %DIR.OMP.LOOP.20, %do.epilog484
  br i1 undef, label %omp.pdo.body477, label %omp.pdo.epilog478

omp.pdo.body477:                                  ; preds = %omp.pdo.cond476
  %do.norm.lb_fetch.391 = load i64, ptr undef, align 8
  store i64 %do.norm.lb_fetch.391, ptr undef, align 8
  br label %do.cond482

do.cond482:                                       ; preds = %do.body483, %omp.pdo.body477
  br i1 undef, label %do.body483, label %do.epilog484

do.body483:                                       ; preds = %do.cond482
  br label %do.cond482

do.epilog484:                                     ; preds = %do.cond482
  br label %omp.pdo.cond476

bb_new479:                                        ; preds = %DIR.OMP.END.MASKED.18
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.COLLAPSE"(i32 2),
    "QUAL.OMP.SCHEDULE.STATIC"(i32 0),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr undef, i64 0, ptr undef, i64 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr undef, i64 0, ptr undef, i64 0) ]
  br label %DIR.OMP.LOOP.20

DIR.OMP.LOOP.20:                                  ; preds = %bb_new479
  %omp.pdo.norm.lb_fetch.381 = load i64, ptr undef, align 8
  store i64 %omp.pdo.norm.lb_fetch.381, ptr undef, align 8
  br label %omp.pdo.cond476

omp.pdo.epilog478:                                ; preds = %omp.pdo.cond476
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.LOOP"() ]
  br label %DIR.OMP.END.LOOP.21

DIR.OMP.END.LOOP.21:                              ; preds = %omp.pdo.epilog478
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.PARALLEL"() ]
  br i1 undef, label %rtn, label %bb32
;  unreachable

rtn:
  ret void

DIR.OMP.PARALLEL.23:                              ; preds = %bb32
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED"(ptr @cccsds_) ]
  br label %bb_new352

bb_new352:                                        ; preds = %DIR.OMP.PARALLEL.23
  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTIONS"() ]
  br label %DIR.OMP.SECTIONS.24

DIR.OMP.SECTIONS.24:                              ; preds = %bb_new352
  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  br label %call.post.list361
}

declare void @llvm.directive.region.exit(token)
declare token @llvm.directive.region.entry()
; end INTEL_CUSTOMIZATION
