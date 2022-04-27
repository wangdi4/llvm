; RUN: opt -vpo-paropt -S %s | FileCheck %s -check-prefix=DEFAULT
; RUN: opt -passes='vpo-paropt' -S %s | FileCheck %s -check-prefix=DEFAULT

; RUN: opt -vpo-paropt -S -vpo-paropt-emit-kmpc-begin-end-only-for-windows=false -vpo-paropt-emit-kmpc-begin=true %s | FileCheck %s -check-prefix=WITH_BEGIN --check-prefix=WITH_END
; RUN: opt -passes='vpo-paropt' -S -vpo-paropt-emit-kmpc-begin-end-only-for-windows=false -vpo-paropt-emit-kmpc-begin=true %s | FileCheck %s -check-prefix=WITH_BEGIN --check-prefix=WITH_END
; RUN: opt -vpo-paropt -S -mtriple=i686-unknown-windows -vpo-paropt-emit-kmpc-begin=true %s | FileCheck %s -check-prefix=WITH_BEGIN --check-prefix=WITH_END
; RUN: opt -passes='vpo-paropt' -S -mtriple=i686-unknown-windows -vpo-paropt-emit-kmpc-begin=true %s | FileCheck %s -check-prefix=WITH_BEGIN --check-prefix=WITH_END
; RUN: opt -vpo-paropt -S -mtriple=x86_64-unknown-windows -vpo-paropt-emit-kmpc-begin=true %s | FileCheck %s -check-prefix=WITH_BEGIN --check-prefix=WITH_END
; RUN: opt -passes='vpo-paropt' -S -mtriple=x86_64-unknown-windows -vpo-paropt-emit-kmpc-begin=true %s | FileCheck %s -check-prefix=WITH_BEGIN --check-prefix=WITH_END

; RUN: opt -vpo-paropt -S -vpo-paropt-emit-kmpc-begin-end-only-for-windows=false %s | FileCheck %s -check-prefix=WITH_END
; RUN: opt -passes='vpo-paropt' -S -vpo-paropt-emit-kmpc-begin-end-only-for-windows=false %s | FileCheck %s -check-prefix=WITH_END
; RUN: opt -vpo-paropt -S -mtriple=i686-unknown-windows %s | FileCheck %s -check-prefix=WITH_END
; RUN: opt -passes='vpo-paropt' -S -mtriple=i686-unknown-windows %s | FileCheck %s -check-prefix=WITH_END
; RUN: opt -vpo-paropt -S -mtriple=x86_64-unknown-windows %s | FileCheck %s -check-prefix=WITH_END
; RUN: opt -passes='vpo-paropt' -S -mtriple=x86_64-unknown-windows %s | FileCheck %s -check-prefix=WITH_END


; extern void bar();
;
; void main_foo() {
; //#pragma omp parallel
;   bar();
; }
;
; int main() {
;   main_foo();
; }

; By default kmpc_begin/end emission is disabled. Only kmpc_end is enabled for
; Windows on the request of libomp team as it is needed for proper invocation
; of OMPT finalization routines.

; DEFAULT-NOT: call void @__kmpc_begin
; DEFAULT-NOT: call void @__kmpc_end

; WITH_BEGIN:    call void @__kmpc_begin(%struct.ident_t* @{{.*}}, i32 0)
; WITH_END:      call void @main_foo()
; WITH_END-NEXT: call void @__kmpc_end(%struct.ident_t* @{{.*}})

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "unknown-unknown-unknown"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  call void @main_foo()
  ret i32 0
}

declare dso_local void @main_foo() #0
declare dso_local void @bar(...) #1

attributes #0 = { noinline nounwind optnone uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 2}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
