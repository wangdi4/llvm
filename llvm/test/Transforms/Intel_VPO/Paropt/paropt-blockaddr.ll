; RUN: opt -S -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt %s | FileCheck %s
; RUN: opt -S -passes="function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),module(vpo-paropt)" %s | FileCheck %s

; Test parallelization of code containing && BlockAddresses.
; The "outside" address should remain in @main, and the "inside" address
; should be outlined to the parallel function.

; CHECK-LABEL: @main
; CHECK: call{{.*}}printf{{.*}}blockaddress(@main, %outside)

; CHECK: define internal void @main.DIR.OMP.PARALLEL
; CHECK: call{{.*}}printf{{.*}}blockaddress(@main.DIR.OMP.PARALLEL{{.*}}, %inside)

;#include <stdio.h>
;int main() {
;  outside:
;    printf("%p\n", && outside);
;  #pragma omp parallel
;  {
;    inside:
;      printf("%p\n", && inside);
;  }
;  return 0;
;}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%p\0A\00", align 1
@"@tid.addr" = external global i32

; Function Attrs: mustprogress norecurse uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  br label %outside

outside:                                          ; preds = %indirectgoto, %entry
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i8* blockaddress(@main, %outside))
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %outside
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  br label %DIR.OMP.PARALLEL.12

DIR.OMP.PARALLEL.12:                              ; preds = %DIR.OMP.PARALLEL.2
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  br label %inside

inside:                                           ; preds = %indirectgoto, %DIR.OMP.PARALLEL.12
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i8* blockaddress(@main, %inside)) #2
  br label %DIR.OMP.END.PARALLEL.3

DIR.OMP.END.PARALLEL.3:                           ; preds = %inside
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.4

DIR.OMP.END.PARALLEL.4:                           ; preds = %DIR.OMP.END.PARALLEL.3
  ret i32 0

indirectgoto:                                     ; No predecessors!
  indirectbr i8* undef, [label %outside, label %inside]
}

declare dso_local i32 @printf(i8*, ...) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { mustprogress norecurse uwtable }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
