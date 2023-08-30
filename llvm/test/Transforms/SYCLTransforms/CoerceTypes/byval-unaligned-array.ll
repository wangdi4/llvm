; RUN: opt -passes=sycl-kernel-coerce-types -mtriple x86_64-pc-linux -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-coerce-types -mtriple x86_64-pc-linux -S %s -o - | FileCheck %s

; Always pad coerced types to normal integer types (i8, i16, i32, i64).

; CHECK: %struct.short7.array.coerce = type { i64, i64 }
%struct.short7.array = type { [7 x i16] } ; 14 bytes

define void @test() {
entry:
  %alloca.struct.short7.array = alloca %struct.short7.array, align 2

; CHECK: call void @short7.array(i64 {{.*}}, i64 {{.*}})
  call void @short7.array(ptr byval(%struct.short7.array) align 2 %alloca.struct.short7.array)
  ret void
}

; CHECK: declare void @short7.array(i64, i64)
declare void @short7.array(ptr byval(%struct.short7.array) align 2)

; DEBUGIFY-NOT: WARNING
