; This test checks caller and callee patching that makes use of the coerced arguments
; RUN: opt -passes='debugify,sycl-kernel-coerce-win64-types,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-coerce-win64-types' -S %s | FileCheck %s

%struct.A = type { i8 }
%struct.B = type { i16 }
%struct.C = type { i32 }
%struct.D = type { i64 }
%struct.E = type { i32, i64 }

; CHECK: define dso_local spir_func void @foo(i8 %0, i16 %1, i32 %2, i64 %3, ptr %4) #1 {
; CHECK-NEXT:  %6 = alloca %struct.E, align 8
; CHECK-NEXT:  %7 = alloca %struct.D, align 8
; CHECK-NEXT:  %8 = alloca %struct.C, align 4
; CHECK-NEXT:  %9 = alloca %struct.B, align 2
; CHECK-NEXT:  %10 = alloca %struct.A, align 1
; CHECK-NEXT:  store i8 %0, ptr %10, align 1
; CHECK-NEXT:  store i16 %1, ptr %9, align 2
; CHECK-NEXT:  store i32 %2, ptr %8, align 4
; CHECK-NEXT:  store i64 %3, ptr %7, align 8
; CHECK-NEXT:  %11 = load i8, ptr %10, align 1
; CHECK-NEXT:  %12 = load i16, ptr %9, align 2
; CHECK-NEXT:  %13 = load i32, ptr %8, align 4
; CHECK-NEXT:  %14 = load i64, ptr %7, align 8
; CHECK-NEXT:  %15 = getelementptr inbounds %struct.E, ptr %6, i32 0
; CHECK-NEXT:  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %15, ptr align 8 %4, i64 12, i1 false)
; CHECK-NEXT:  call spir_func void @foo(i8 %11, i16 %12, i32 %13, i64 %14, ptr %6) #3
; CHECK-NEXT:  ret void
define dso_local spir_func void @foo(ptr byval(%struct.A) align 1 %0, ptr byval(%struct.B) align 2 %1, ptr byval(%struct.C) align 4 %2, ptr byval(%struct.D) align 8 %3, ptr byval(%struct.E) align 8 %4) #0 {
  call spir_func void @foo(ptr byval(%struct.A) align 1 %0, ptr byval(%struct.B) align 2 %1, ptr byval(%struct.C) align 4 %2, ptr byval(%struct.D) align 8 %3, ptr byval(%struct.E) align 8 %4) #2
  ret void
}

; Make sure alloca memory with right addrspace.
define void @foo1(ptr addrspace(4) byval(%struct.D) align 8 %arg) #0 {
; CHECK: define void @foo1(i64 %arg) #1 {
; CHECK:      %1 = alloca %struct.D, align 8
; CHECK-NEXT: %2 = addrspacecast ptr %1 to ptr addrspace(4)
 ret void
}


declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { convergent noinline norecurse nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable willreturn }
attributes #2 = { convergent }

; DEBUGIFY-NOT: WARNING
