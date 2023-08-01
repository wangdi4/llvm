; REQUIRES: asserts
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug -S %s 2>&1 | FileCheck %s

;Test src:
;
;void foo() {
;double x = 0.0;
;double *out = &x;
;int n,m,o, k;
;#pragma omp target data map(tofrom:out[0:1]) device(m) subdevice (1, 2:n:4)
;    out[0] = 123.0;
; }
;
;int main(){
;foo();
;return 0;
;}

; check debug message:
; CHECK:  Subdevice encoding : Device is not constant.
; CHECK:  Subdevice encoding : Level before shift: 0x0000000000000001, after shift: 0x0100000000000000
; CHECK:  Subdevice encoding : Start before shift: 0x0000000000000002, after shift: 0x0002000000000000
; CHECK:  Subdevice encoding : Length is not constant.
; CHECK:  Subdevice encoding : Stride before shift: 0x0000000000000004, after shift: 0x0000000400000000
;
; check the generated code for non constants params .
; CHECK:  [[DeviceZext:%[a-zA-Z._0-9]+]] = zext i32 %{{.*}} to i64
; CHECK:  [[DeviceMasked:%[a-zA-Z._0-9]+]] = and i64 [[DeviceZext]], 4294967295
; CHECK:  [[DeviceShifted:%[a-zA-Z._0-9]+]] = shl i64 [[DeviceMasked]], 0
; CHECK:  [[LengthZext:%[a-zA-Z._0-9]+]] = zext i32 %{{.*}} to i64
; CHECK:  [[LengthMasked:%[a-zA-Z._0-9]+]] = and i64 [[LengthZext]], 255
; CHECK:  [[LengthShifted:%[a-zA-Z._0-9]+]] = shl i64 [[LengthMasked]], 40
;
; The value -9150751475683557376 is the initial Encoding containing the constant params
; CHECK:  [[OR1:%[a-zA-Z._0-9]+]] = or i64 [[DeviceShifted]], -9150751475683557376
; CHECK:  [[DeviceID:%[a-zA-Z._0-9]+]] = or i64 [[LengthShifted]], [[OR1]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local void @foo() {
entry:
  %x = alloca double, align 8
  %out = alloca double*, align 8
  %n = alloca i32, align 4
  %m = alloca i32, align 4
  %o = alloca i32, align 4
  %k = alloca i32, align 4
  store double 0.000000e+00, double* %x, align 8
  store double* %x, double** %out, align 8
  %0 = load i32, i32* %m, align 4
  %1 = load i32, i32* %n, align 4
  %2 = load double*, double** %out, align 8
  %3 = load double*, double** %out, align 8
  %arrayidx = getelementptr inbounds double, double* %3, i64 0
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(),
    "QUAL.OMP.DEVICE"(i32 %0),
    "QUAL.OMP.SUBDEVICE"(i32 1, i32 2, i32 %1, i32 4),
    "QUAL.OMP.MAP.TOFROM"(double* %2, double* %arrayidx, i64 8, i64 35) ]

  %5 = load double*, double** %out, align 8
  %ptridx = getelementptr inbounds double, double* %5, i64 0
  store double 1.230000e+02, double* %ptridx, align 8
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TARGET.DATA"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  call void @foo()
  ret i32 0
}
