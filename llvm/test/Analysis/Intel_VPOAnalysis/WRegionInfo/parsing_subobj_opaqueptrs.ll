; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S -disable-output %s 2>&1 | FileCheck %s
;
; Test src:
; typedef struct {short x; } S;
; short a, b, c, d, e, f, g, h, i;
; S j;
; short *p;
; #pragma omp declare target to(g)
; void f1(void) {
;
; #pragma omp target map(a)
;   g;
;
; #pragma omp parallel for private(b) firstprivate(c) lastprivate(d) shared(e) linear(f:2) reduction(+:h) private(j)
;   for (i = 0; i < 10; i++);
;
; #pragma omp target data use_device_ptr(p)
;   ;
; }
;
; The SUBOBJ modifier was added by hand-modifying the IR for the above test.
; CHECK:    BEGIN TARGET ID=1 {
; CHECK:      LIVEIN clause (size=1): SUBOBJ(ptr @g)
; CHECK:      MAP clause (size=1): CHAIN,SUBOBJ(<ptr @a, ptr @a, i64 2, 3 (0x0000000000000003), null, null> )
; CHECK:    } END TARGET ID=1
;
;
; CHECK:    BEGIN PARALLEL.LOOP ID=2 {
; CHECK:      SHARED clause (size=1): SUBOBJ,TYPED(ptr @e, TYPE: i16, NUM_ELEMENTS: i32 1)
; CHECK:      PRIVATE clause (size=3): SUBOBJ,TYPED(ptr @b, TYPE: i16, NUM_ELEMENTS: i32 1)
; CHECK-SAME:                          TYPED(ptr @i, TYPE: i16, NUM_ELEMENTS: i32 1)
; CHECK-SAME:                          NONPOD(SUBOBJ(TYPED(ptr @j, TYPE: %struct.S = type { i16 }, NUM_ELEMENTS: i32 1)), CTOR: UNSPECIFIED, DTOR: UNSPECIFIED)
; CHECK:      FIRSTPRIVATE clause (size=2): SUBOBJ,TYPED(ptr @c, TYPE: i16, NUM_ELEMENTS: i32 1) SUBOBJ,TYPED(ptr %.omp.lb, TYPE: i32, NUM_ELEMENTS: i32 1)
; CHECK:      LASTPRIVATE clause (size=1): SUBOBJ,TYPED(ptr @d, TYPE: i16, NUM_ELEMENTS: i32 1)
; CHECK:      REDUCTION clause (size=1): (ADD: SUBOBJ(TYPED(ptr @h, TYPE: i16, NUM_ELEMENTS: i32 1)))
; CHECK:      LINEAR clause (size=1): (SUBOBJ(TYPED(ptr @f, TYPE: i16, NUM_ELEMENTS: i32 1)), i32 2)
; CHECK:    } END PARALLEL.LOOP ID=2

; CHECK:    BEGIN TARGET.DATA ID=3 {
; CHECK:      USE_DEVICE_PTR clause (size=1): SUBOBJ,PTR_TO_PTR(ptr @p)
; CHECK:    } END TARGET.DATA ID=3

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

%struct.S = type { i16 }

@g = dso_local target_declare global i16 0, align 2
@a = dso_local global i16 0, align 2
@b = dso_local global i16 0, align 2
@c = dso_local global i16 0, align 2
@d = dso_local global i16 0, align 2
@e = dso_local global i16 0, align 2
@f = dso_local global i16 0, align 2
@h = dso_local global i16 0, align 2
@i = dso_local global i16 0, align 2
@j = dso_local global %struct.S zeroinitializer, align 2
@p = dso_local global ptr null, align 8

define dso_local void @f1() {
entry:
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1),
    "QUAL.OMP.MAP.TOFROM:SUBOBJ"(ptr @a, ptr @a, i64 2, i64 3, ptr null, ptr null), ; MAP type: 3 = 0x3 = FROM (0x2) | TO (0x1)
    "QUAL.OMP.LIVEIN:SUBOBJ"(ptr @g) ]

  %1 = load i16, ptr @g, align 2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  store i32 0, ptr %.omp.lb, align 4
  store i32 9, ptr %.omp.ub, align 4
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.PRIVATE:TYPED.SUBOBJ"(ptr @b, i16 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED.SUBOBJ"(ptr @c, i16 0, i32 1),
    "QUAL.OMP.LASTPRIVATE:TYPED.SUBOBJ"(ptr @d, i16 0, i32 1),
    "QUAL.OMP.SHARED:TYPED.SUBOBJ"(ptr @e, i16 0, i32 1),
    "QUAL.OMP.LINEAR:TYPED.SUBOBJ"(ptr @f, i16 0, i32 1, i32 2),
    "QUAL.OMP.REDUCTION.ADD:TYPED.SUBOBJ"(ptr @h, i16 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr @i, i16 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED.SUBOBJ.NONPOD"(ptr @j, %struct.S zeroinitializer, i32 1, ptr null, ptr null),
    "QUAL.OMP.NORMALIZED.IV:TYPED.SUBOBJ"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED.SUBOBJ"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED.SUBOBJ"(ptr %.omp.ub, i32 0) ]

  %3 = load i32, ptr %.omp.lb, align 4
  store i32 %3, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, ptr %.omp.iv, align 4
  %5 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  %conv = trunc i32 %add to i16
  store i16 %conv, ptr @i, align 2
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %7, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(),
    "QUAL.OMP.USE_DEVICE_PTR:PTR_TO_PTR.SUBOBJ"(ptr @p) ]

  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TARGET.DATA"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0, !1}
!0 = !{i32 0, i32 66306, i32 7826497, !"_Z2f1", i32 4, i32 0, i32 1, i32 0}
!1 = !{i32 1, !"_Z1g", i32 0, i32 0, ptr @g}
