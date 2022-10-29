; REQUIRES: asserts
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction-atomic=false -disable-output -debug-only=vpo-paropt-transform -S %s 2>&1 | FileCheck %s

; Test src:
;
; void f1(long n) { // IR hand-modified, simplified
;   short a[n];
; #pragma omp parallel reduction(+:a)
;   ;
; }
;
; void f2() {
;   short b;
; #pragma omp parallel reduction(+:b)
;   ;
; }
;
; void f3() {
;   short c[10];
; #pragma omp parallel reduction(+:c)
;   ;
; }
;
; void f4() {
;   __int128 d;
; #pragma omp parallel reduction(+:d)
;   ;
; }
;
; void f5() {
;   _Quad f;
; #pragma omp parallel reduction(+:f)
;   ;
; }
;
; void f6(_Quad *g) {
; #pragma omp parallel reduction(+:g[0])
;   ;
; }
;
; void f7(_Quad *h) {
; #pragma omp parallel reduction(+:h[0]) // IR hand-modified to not use ARRSECT
;   ;
; }
;
; void f8() {
;   _Quad i;
;   _Quad &j = i;
; #pragma omp parallel reduction(+:j)
;   ;
; }
;
; void f9() {
;   short k[10];
; #pragma omp parallel reduction(+:k[0:5])
;   ;
; }
;
; void f10(long n) {
;   short l[10];
; #pragma omp parallel reduction(+:l[0:n])
;   ;
; }

; Check how the alignment for various fast reduction structs is determined.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; For VLAs, the struct gets a pointer, so the alignment should be based on
; pointer type, not the alignment of the original VLA.
; CHECK: genFastRedTyAndVar: RedI: i16* %a, ElemTy (i16*)'s preferred align: 8, fast_red_struct's align: 8
; CHECK: genFastRedTyAndVar: Create alloca for fast reduction structure::   %fast_red_struct = alloca %struct.fast_red_t, align 8, type=%struct.fast_red_t = type <{ i16* }>
define dso_local void @_Z2f1l(i64 noundef %n) {
entry:
  %a = alloca i16, i64 %n, align 16
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.REDUCTION.ADD"(i16* %a) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

; Minimum alignment we use, same as that of an empty struct, is 8.
; CHECK: genFastRedTyAndVar: RedI: i16* %b, orig's align: 2, ElemTy (i16)'s preferred align: 2, fast_red_struct's align: 8
; CHECK: genFastRedTyAndVar: Create alloca for fast reduction structure::   %fast_red_struct = alloca %struct.fast_red_t.0, align 8, type=%struct.fast_red_t.0 = type <{ i16 }>
define dso_local void @_Z2f2v() {
entry:
  %b = alloca i16, align 2
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.REDUCTION.ADD"(i16* %b) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

; The original array's alignment, 16, is larger than the preferred alignment, 2.
; So 16 is used.
; CHECK: genFastRedTyAndVar: RedI: [10 x i16]* %c, orig's align: 16, ElemTy ([10 x i16])'s preferred align: 2, fast_red_struct's align: 16
; CHECK: genFastRedTyAndVar: Create alloca for fast reduction structure::   %fast_red_struct = alloca %struct.fast_red_t.1, align 16, type=%struct.fast_red_t.1 = type <{ [10 x i16] }>
define dso_local void @_Z2f3v() {
entry:
  %c = alloca [10 x i16], align 16
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.REDUCTION.ADD"([10 x i16]* %c) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

; The original item's alignment, 16, is larger than the preferred alignment, 8.
; So 16 is used.
; CHECK: genFastRedTyAndVar: RedI: i128* %d, orig's align: 16, ElemTy (i128)'s preferred align: 8, fast_red_struct's align: 16
; CHECK: genFastRedTyAndVar: Create alloca for fast reduction structure::   %fast_red_struct = alloca %struct.fast_red_t.2, align 16, type=%struct.fast_red_t.2 = type <{ i128 }>
define dso_local void @_Z2f4v() {
entry:
  %d = alloca i128, align 16
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.REDUCTION.ADD"(i128* %d) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

; CHECK: genFastRedTyAndVar: RedI: fp128* %f, orig's align: 16, ElemTy (fp128)'s preferred align: 16, fast_red_struct's align: 16
; CHECK: genFastRedTyAndVar: Create alloca for fast reduction structure::   %fast_red_struct = alloca %struct.fast_red_t.3, align 16, type=%struct.fast_red_t.3 = type <{ fp128 }>
define dso_local void @_Z2f5v() {
entry:
  %f = alloca fp128, align 16
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.REDUCTION.ADD"(fp128* %f) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

; For array-section on pointers, the struct's alignment is determined based on
; the preferred alignment of the element-type (16 for [ 1 x fp128]) and not the
; base pointer (8).
; CHECK: genFastRedTyAndVar: RedI: fp128** %g.addr, ElemTy ([1 x fp128])'s preferred align: 16, fast_red_struct's align: 16
; CHECK: genFastRedTyAndVar: Create alloca for fast reduction structure::   %fast_red_struct = alloca %struct.fast_red_t.4, align 16, type=%struct.fast_red_t.4 = type <{ [1 x fp128] }>
define dso_local void @_Z2f6Pg(fp128* noundef %g) {
entry:
  %g.addr = alloca fp128*, align 8
  store fp128* %g, fp128** %g.addr, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT"(fp128** %g.addr, i64 1, i64 0, i64 1, i64 1) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

; For clause operands that are load instructions, Value::getPointerAlignment
; returns 1, so looking at the element type's alignment is important.
; CHECK: genFastRedTyAndVar: RedI: fp128* %h.load, orig's align: 1, ElemTy (fp128)'s preferred align: 16, fast_red_struct's align: 16
; CHECK: genFastRedTyAndVar: Create alloca for fast reduction structure::   %fast_red_struct = alloca %struct.fast_red_t.5, align 16, type=%struct.fast_red_t.5 = type <{ fp128 }>
define dso_local void @_Z2f7Pg(fp128* noundef %h) {
entry:
  %h.addr = alloca fp128*, align 8
  store fp128* %h, fp128** %h.addr, align 8
  %h.load = load fp128*, fp128** %h.addr, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.REDUCTION.ADD"(fp128* %h.load) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

; For BYREFs, Orig's alignment will always be that of a pointer, 8. So element
; type's alignment should be used.
; CHECK: genFastRedTyAndVar: RedI: fp128** %j, ElemTy (fp128)'s preferred align: 16, fast_red_struct's align: 16
; CHECK: genFastRedTyAndVar: Create alloca for fast reduction structure::   %fast_red_struct = alloca %struct.fast_red_t.6, align 16, type=%struct.fast_red_t.6 = type <{ fp128 }>
define dso_local void @_Z2f8v() {
entry:
  %i = alloca fp128, align 16
  %j = alloca fp128*, align 8
  store fp128* %i, fp128** %j, align 8
  %0 = load fp128*, fp128** %j, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.REDUCTION.ADD:BYREF"(fp128** %j) ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

; Base array's alignment is 16, but the preferred alignment is 2. We use 16
; even for a section of that array.
; CHECK: genFastRedTyAndVar: RedI: [10 x i16]* %k, orig's align: 16, ElemTy ([5 x i16])'s preferred align: 2, fast_red_struct's align: 16
; CHECK: genFastRedTyAndVar: Create alloca for fast reduction structure::   %fast_red_struct = alloca %struct.fast_red_t.7, align 16, type=%struct.fast_red_t.7 = type <{ [5 x i16] }>
define dso_local void @_Z2f9v() #0 {
entry:
  %k = alloca [10 x i16], align 16
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT"([10 x i16]* %k, i64 1, i64 0, i64 5, i64 1) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

; For variable length array-sections, similar to VLAs, the struct member is
; a pointer to the element type i16. So, instead of the base array's alignment,
; 16, we use the pointer type (i16*)'s preferred alignment, 8.
; CHECK: genFastRedTyAndVar: RedI: [10 x i16]* %l, ElemTy (i16*)'s preferred align: 8, fast_red_struct's align: 8
; CHECK: genFastRedTyAndVar: Create alloca for fast reduction structure::   %fast_red_struct = alloca %struct.fast_red_t.8, align 8, type=%struct.fast_red_t.8 = type <{ i16* }>
define dso_local void @_Z3f10l(i64 noundef %n) #0 {
entry:
  %n.addr = alloca i64, align 8
  %l = alloca [10 x i16], align 16
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT"([10 x i16]* %l, i64 1, i64 0, i64 %n, i64 1) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
