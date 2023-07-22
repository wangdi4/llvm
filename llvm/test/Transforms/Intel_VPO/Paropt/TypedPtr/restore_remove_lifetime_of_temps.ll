; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-restore-operands -S %s | FileCheck %s -check-prefixes=RESTR,ALL
; RUN: opt -opaque-pointers=0 -passes='function(vpo-restore-operands)' -S %s | FileCheck %s -check-prefixes=RESTR,ALL
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefixes=TFORM,ALL
; RUN: opt -opaque-pointers=0 -passes='function(vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefixes=TFORM,ALL

; Test src:

;  int y;
;  int b() {
;  #pragma omp parallel firstprivate(a)
;    y = 111;
;  }
;  int c() { b(); }

; The IR is the output of inliner, which inserts lifetime begin/end intrinsics
; for temp vars generated in prepare pass.

; Additional lifetime end markers were added for %y.addr.i to make sure we
; can handle deletion of multiple markers for the same operand addr.

; ALL-NOT:       %y.addr.i

; RESTR-COUNT-1: call void @llvm.lifetime.start.p0i8
; RESTR-COUNT-1: call void @llvm.lifetime.end.p0i8

; TFORM-NOT:     %end.dir.temp.i
; TFORM-NOT:     call void @llvm.lifetime.start.p0i8
; TFORM-NOT:     call void @llvm.lifetime.end.p0i8

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@y = dso_local global i32 0, align 4

define dso_local i32 @c() local_unnamed_addr {
entry:
  %y.addr.i = alloca i32*, align 8
  %end.dir.temp.i = alloca i1, align 1
  %0 = bitcast i32** %y.addr.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %0)
  %1 = bitcast i1* %end.dir.temp.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %1)
  store i32* @y, i32** %y.addr.i, align 8

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
  "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* @y, i32 0, i32 1),
  "QUAL.OMP.OPERAND.ADDR"(i32* @y, i32** %y.addr.i),
  "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp.i) ]

  %temp.load.i = load volatile i1, i1* %end.dir.temp.i, align 1
  br i1 %temp.load.i, label %b.exit, label %DIR.OMP.PARALLEL.3.i

DIR.OMP.PARALLEL.3.i:                             ; preds = %entry
  %y.i = load volatile i32*, i32** %y.addr.i, align 8
  store i32 111, i32* %y.i, align 4
  br label %b.exit

b.exit:                                           ; preds = %entry, %DIR.OMP.PARALLEL.3.i
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]
  %3 = bitcast i32** %y.addr.i to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %3)

  call void @llvm.lifetime.end.p0i8(i64 8, i8* %3)
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %3)

  %4 = bitcast i1* %end.dir.temp.i to i8*
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %4)
  ret i32 undef
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)
