; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR
;
; Test src:
;
;
; #include <string.h>
; #include <stdio.h>
;
; void print_int (void *c);
;
; int main() {
;
;   int y = 1;
;   print_int((void*) &y);
;
; #pragma omp parallel num_threads(1) firstprivate(y)
;   {
;     print_int((void*) &y);
;   }
;
;   return 0;
; }

; This test is a subset of rename_and_restore_int_addrcast.ll, which captures the IR
; after instcombine, and does not depend on the optimization's behavior to
; stay the same (for example, the bitcast on %y1 is no longer present with opaque
; pointers).

; Check for restore-operands was able to undo the renaming:
; RESTR: "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* %y,
; RESTR-NOT: alloca i32*
; RESTR-NOT: "QUAL.OMP.OPERAND.ADDR"
; RESTR: [[YRENAMED:%[a-zA-Z._0-9]+]] = bitcast i32* %y to i8*
; RESTR: call void @print_int(i8* [[YRENAMED]])

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"@tid.addr" = external global i32

define dso_local i32 @main() {
entry:
  %y = alloca i32, align 4
  %0 = bitcast i32* %y to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0)
  store i32 1, i32* %y, align 4, !tbaa !4
  call void @print_int(i8* nonnull %0)
  %y.addr = alloca i32*, align 8
  store i32* %y, i32** %y.addr, align 8
  %end.dir.temp = alloca i1, align 1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.NUM_THREADS"(i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* %y, i32 0, i32 1),
    "QUAL.OMP.OPERAND.ADDR"(i32* %y, i32** %y.addr),
    "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]

  %temp.load = load volatile i1, i1* %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.PARALLEL.4.split, label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %entry
  %y1 = load volatile i32*, i32** %y.addr, align 8
  %2 = bitcast i32* %y1 to i8*
  call void @print_int(i8* %2)
  br label %DIR.OMP.END.PARALLEL.4.split

DIR.OMP.END.PARALLEL.4.split:                     ; preds = %DIR.OMP.PARALLEL.3, %entry
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]

  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0)
  ret i32 0
}

declare dso_local void @print_int(i8*)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{!"clang version 17.0.0"}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
