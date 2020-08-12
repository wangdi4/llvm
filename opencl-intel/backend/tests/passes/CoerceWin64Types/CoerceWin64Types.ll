; This test checks caller and callee patching that makes use of the coerced arguments
; RUN: %oclopt -coerce-win64-types -mtriple x86_64-w64-mingw32 -S %s -o - | FileCheck %s

%struct.A = type { i8 }
%struct.B = type { i16 }
%struct.C = type { i32 }
%struct.D = type { i64 }
%struct.E = type { i32, i64 }

; CHECK: define dso_local spir_func void @foo(i8 %0, i16 %1, i32 %2, i64 %3, %struct.E* %4) #1 {
; CHECK-NEXT:   %6 = alloca %struct.E, align 8
; CHECK-NEXT:   %7 = alloca %struct.D, align 8
; CHECK-NEXT:   %8 = alloca %struct.C, align 4
; CHECK-NEXT:   %9 = alloca %struct.B, align 2
; CHECK-NEXT:   %10 = alloca %struct.A, align 1
; CHECK-NEXT:   %11 = getelementptr inbounds %struct.A, %struct.A* %10, i32 0
; CHECK-NEXT:   %12 = bitcast %struct.A* %11 to i8*
; CHECK-NEXT:   store i8 %0, i8* %12, align 1
; CHECK-NEXT:   %13 = getelementptr inbounds %struct.B, %struct.B* %9, i32 0
; CHECK-NEXT:   %14 = bitcast %struct.B* %13 to i16*
; CHECK-NEXT:   store i16 %1, i16* %14, align 2
; CHECK-NEXT:   %15 = getelementptr inbounds %struct.C, %struct.C* %8, i32 0
; CHECK-NEXT:   %16 = bitcast %struct.C* %15 to i32*
; CHECK-NEXT:   store i32 %2, i32* %16, align 4
; CHECK-NEXT:   %17 = getelementptr inbounds %struct.D, %struct.D* %7, i32 0
; CHECK-NEXT:   %18 = bitcast %struct.D* %17 to i64*
; CHECK-NEXT:   store i64 %3, i64* %18, align 8
; CHECK-NEXT:   %19 = getelementptr inbounds %struct.A, %struct.A* %10, i32 0
; CHECK-NEXT:   %20 = bitcast %struct.A* %19 to i8*
; CHECK-NEXT:   %21 = load i8, i8* %20, align 1
; CHECK-NEXT:   %22 = getelementptr inbounds %struct.B, %struct.B* %9, i32 0
; CHECK-NEXT:   %23 = bitcast %struct.B* %22 to i16*
; CHECK-NEXT:   %24 = load i16, i16* %23, align 2
; CHECK-NEXT:   %25 = getelementptr inbounds %struct.C, %struct.C* %8, i32 0
; CHECK-NEXT:   %26 = bitcast %struct.C* %25 to i32*
; CHECK-NEXT:   %27 = load i32, i32* %26, align 4
; CHECK-NEXT:   %28 = getelementptr inbounds %struct.D, %struct.D* %7, i32 0
; CHECK-NEXT:   %29 = bitcast %struct.D* %28 to i64*
; CHECK-NEXT:   %30 = load i64, i64* %29, align 8
; CHECK-NEXT:   %31 = getelementptr inbounds %struct.E, %struct.E* %6, i32 0
; CHECK-NEXT:   %32 = bitcast %struct.E* %31 to i8*
; CHECK-NEXT:   %33 = bitcast %struct.E* %4 to i8*
; CHECK-NEXT:   call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %32, i8* align 8 %33, i64 12, i1 false)
; CHECK-NEXT:   call spir_func void @foo(i8 %21, i16 %24, i32 %27, i64 %30, %struct.E* %6) #3
; CHECK-NEXT:   ret void

define dso_local spir_func void @foo(%struct.A* byval(%struct.A) align 1 %0, %struct.B* byval(%struct.B) align 2 %1, %struct.C* byval(%struct.C) align 4 %2, %struct.D* byval(%struct.D) align 8 %3, %struct.E* byval(%struct.E) align 8 %4) #0 {
  call spir_func void @foo(%struct.A* byval(%struct.A) align 1 %0, %struct.B* byval(%struct.B) align 2 %1, %struct.C* byval(%struct.C) align 4 %2, %struct.D* byval(%struct.D) align 8 %3, %struct.E* byval(%struct.E) align 8 %4) #2
  ret void
}

declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { convergent noinline norecurse nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable willreturn }
attributes #2 = { convergent }


