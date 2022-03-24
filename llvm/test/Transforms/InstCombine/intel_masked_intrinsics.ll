; RUN: opt -instcombine -S < %s | FileCheck %s
; RUN: opt -instcombine -opaque-pointers -S < %s | FileCheck %s

; Define variables for typed or opaque pointers
; CHECK-LABEL: %pointers = type
; CHECK-SAME: { [[DOUBLE_P:[^,]*]], [[SDOUBLE_P:.*]] }
%pointers = type { double*, {double, double}* }
@pointers = global %pointers zeroinitializer

declare void @llvm.masked.scatter.v2f64.v2p0f64(<2 x double> %val, <2 x double*> %ptrs, i32, <2 x i1> %mask)

define void @scatter_splat_pointer(double* %ptr, <2 x double> %val, <2 x i1> %mask)  {
; CHECK-LABEL: @scatter_splat_pointer(
; CHECK-NOT: insertelement
; CHECK-NOT: shufflevector
; CHECK:    call void @llvm.masked.scatter.v2f64.{{.*}}(<2 x double> [[VAL:%.*]], <2 x [[DOUBLE_P]]> [[ADDR:%.*]], i32 8, <2 x i1> [[MASK:%.*]])
; CHECK-NEXT:    ret void
;
  %.splatinsert = insertelement <2 x double*> undef, double* %ptr, i64 0
  %.splat = shufflevector <2 x double*> %.splatinsert, <2 x double*> undef, <2 x i32> zeroinitializer
  call void @llvm.masked.scatter.v2f64.v2p0f64(<2 x double> %val, <2 x double*> %.splat, i32 8, <2 x i1> %mask)
  ret void
}

; We can remove an alloca this only stored to.
define void @scatter_alloca(<2 x double> %val, <2 x i1> %mask)  {
; CHECK-LABEL: @scatter_alloca(
; CHECK-NEXT:    ret void
;
  %ptr = alloca double, align 8
  %.splatinsert = insertelement <2 x double*> undef, double* %ptr, i64 0
  %.splat = shufflevector <2 x double*> %.splatinsert, <2 x double*> undef, <2 x i32> zeroinitializer
  call void @llvm.masked.scatter.v2f64.v2p0f64(<2 x double> %val, <2 x double*> %.splat, i32 8, <2 x i1> %mask)
  ret void
}

define void @scatter_splat_struct_gep({ double, double }* %ptr, <2 x double> %val, <2 x i1> %mask)  {
; CHECK-LABEL: @scatter_splat_struct_gep(
; CHECK-NEXT:    [[GEP:%.*]] = getelementptr { double, double }, [[SDOUBLE_P]] [[PTR:%.*]], i64 0, i32 1
; CHECK-NEXT:    [[TMP1:%.*]] = getelementptr double, [[DOUBLE_P]] [[GEP]], <2 x i64> zeroinitializer
; CHECK-NEXT:    call void @llvm.masked.scatter.v2f64.{{.*}}(<2 x double> [[VAL:%.*]], <2 x [[DOUBLE_P]]> [[TMP1]], i32 8, <2 x i1> [[MASK:%.*]])
; CHECK-NEXT:    ret void
;
  %gep = getelementptr { double, double }, { double, double }* %ptr, i32 0, i32 1
  %.splatinsert = insertelement <2 x double*> undef, double* %gep, i64 0
  %.splat = shufflevector <2 x double*> %.splatinsert, <2 x double*> undef, <2 x i32> zeroinitializer
  call void @llvm.masked.scatter.v2f64.v2p0f64(<2 x double> %val, <2 x double*> %.splat, i32 8, <2 x i1> %mask)
  ret void
}
