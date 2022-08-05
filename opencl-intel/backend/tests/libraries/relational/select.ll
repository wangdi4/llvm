; RUN: llvm-extract %libdir/../clbltfnshared.rtl -rfunc '_Z6select.*Dh' -rfunc '_Z14select_v1widenDv64_Dh' -S -o %t
; RUN: FileCheck %s --input-file=%t --check-prefixes=CHECK-SCALAR,CHECK-COMMON
; RUN: FileCheck %s --input-file=%t --check-prefixes=CHECK-V1WIDEN,CHECK-COMMON
; RUN: FileCheck %s --input-file=%t --check-prefixes=CHECK-VECTOR,CHECK-COMMON

; Example scalar implementation:
; CHECK-SCALAR: define {{.*}} half @_Z6selectDhDht(half{{.*}} %a, half{{.*}} %b, i16{{.*}} %c)
; CHECK-SCALAR-NEXT: entry:
; CHECK-SCALAR-NEXT: [[ISZERO:%.*]] = icmp eq i16 %c, 0
; CHECK-SCALAR-NEXT: [[SELECT:%.*]] = select {{.*}} i1 [[ISZERO]], half %a, half %b
; CHECK-SCALAR-NEXT: ret half [[SELECT]]

; Example widened variant for scalar implementation:
; CHECK-V1WIDEN: define {{.*}} <64 x half> @_Z14select_v1widenDv64_DhS_Dv64_s(<64 x half> {{.*}} %a, <64 x half> {{.*}} %b, <64 x i16> {{.*}} %c)
; CHECK-V1WIDEN-NEXT: entry:
; CHECK-V1WIDEN-NEXT: [[ISZERO:%.*]] = icmp eq <64 x i16> %c, zeroinitializer
; CHECK-V1WIDEN-NEXT: [[SELECT:%.*]] = select <64 x i1> [[ISZERO]], <64 x half> %a, <64 x half> %b
; CHECK-V1WIDEN-NEXT: ret <64 x half> [[SELECT]]

; Example vector implementation:
; CHECK-VECTOR: define {{.*}} <64 x half> @_Z6selectDv64_DhS_Dv64_t(<64 x half>{{.*}} %a, <64 x half>{{.*}} %b, <64 x i16>{{.*}} %c)
; CHECK-VECTOR-NEXT: entry:
; CHECK-VECTOR-NEXT: [[ISNEG:%.*]] = icmp slt <64 x i16> %c, zeroinitializer
; CHECK-VECTOR-NEXT: [[SELECT:%.*]] = select <64 x i1> [[ISNEG]], <64 x half> %b, <64 x half> %a
; CHECK-VECTOR-NEXT: ret <64 x half> [[SELECT]]

; Checks we don't invovle any inefficient shuffling in all select implementations.
; CHECK-COMMON-NOT: shufflevector
