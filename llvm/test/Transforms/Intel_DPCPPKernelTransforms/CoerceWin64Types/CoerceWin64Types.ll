; This test checks caller and callee patching that makes use of the coerced arguments
; RUN: opt -passes='debugify,dpcpp-kernel-coerce-win64-types,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='dpcpp-kernel-coerce-win64-types' -S %s | FileCheck %s

%struct.A = type { i8 }
%struct.B = type { i16 }
%struct.C = type { i32 }
%struct.D = type { i64 }
%struct.E = type { i32, i64 }

; CHECK: define dso_local spir_func void @foo(i8 %0, i16 %1, i32 %2, i64 %3, %struct.E* %4) #1 {
; CHECK-NEXT:  %6 = alloca %struct.E, align 8
; CHECK-NEXT:  %7 = alloca %struct.D, align 8
; CHECK-NEXT:  %8 = alloca %struct.C, align 4
; CHECK-NEXT:  %9 = alloca %struct.B, align 2
; CHECK-NEXT:  %10 = alloca %struct.A, align 1
; CHECK-NEXT:  %11 = bitcast %struct.A* %10 to i8*
; CHECK-NEXT:  store i8 %0, i8* %11, align 1
; CHECK-NEXT:  %12 = bitcast %struct.B* %9 to i16*
; CHECK-NEXT:  store i16 %1, i16* %12, align 2
; CHECK-NEXT:  %13 = bitcast %struct.C* %8 to i32*
; CHECK-NEXT:  store i32 %2, i32* %13, align 4
; CHECK-NEXT:  %14 = bitcast %struct.D* %7 to i64*
; CHECK-NEXT:  store i64 %3, i64* %14, align 8
; CHECK-NEXT:  %15 = bitcast %struct.A* %10 to i8*
; CHECK-NEXT:  %16 = load i8, i8* %15, align 1
; CHECK-NEXT:  %17 = bitcast %struct.B* %9 to i16*
; CHECK-NEXT:  %18 = load i16, i16* %17, align 2
; CHECK-NEXT:  %19 = bitcast %struct.C* %8 to i32*
; CHECK-NEXT:  %20 = load i32, i32* %19, align 4
; CHECK-NEXT:  %21 = bitcast %struct.D* %7 to i64*
; CHECK-NEXT:  %22 = load i64, i64* %21, align 8
; CHECK-NEXT:  %23 = getelementptr inbounds %struct.E, %struct.E* %6, i32 0
; CHECK-NEXT:  %24 = bitcast %struct.E* %23 to i8*
; CHECK-NEXT:  %25 = bitcast %struct.E* %4 to i8*
; CHECK-NEXT:  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %24, i8* align 8 %25, i64 12, i1 false)
; CHECK-NEXT:  call spir_func void @foo(i8 %16, i16 %18, i32 %20, i64 %22, %struct.E* %6) #3
; CHECK-NEXT:  ret void
define dso_local spir_func void @foo(%struct.A* byval(%struct.A) align 1 %0, %struct.B* byval(%struct.B) align 2 %1, %struct.C* byval(%struct.C) align 4 %2, %struct.D* byval(%struct.D) align 8 %3, %struct.E* byval(%struct.E) align 8 %4) #0 {
  call spir_func void @foo(%struct.A* byval(%struct.A) align 1 %0, %struct.B* byval(%struct.B) align 2 %1, %struct.C* byval(%struct.C) align 4 %2, %struct.D* byval(%struct.D) align 8 %3, %struct.E* byval(%struct.E) align 8 %4) #2
  ret void
}

; Make sure alloca memory with right addrspace.
define void @foo1(%struct.D addrspace(4)* byval(%struct.D) %arg) #0 {
; CHECK: define void @foo1(i64 %arg) #1 {
; CHECK:      %1 = alloca %struct.D, align 8
; CHECK-NEXT: %2 = addrspacecast %struct.D* %1 to %struct.D addrspace(4)*
 ret void
}


declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { convergent noinline norecurse nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable willreturn }
attributes #2 = { convergent }

; DEBUGIFY-NOT: WARNING
