; RUN: opt < %s -opaque-pointers -dope-vector-local-const-prop=false -S -passes=dopevectorconstprop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

; Test dope vector analysis and constant propagation when uplevel
; variables are passed to a recursive function.
; This is the same test as dvcp07.ll, but checks the IR rather than the traces.
; Test case uses 1 rank dope vector which looks like:
; { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
; Uplevel variable will consist of a single dope vector

%uplevel_type = type { ptr }

define dso_local void @MAIN__() #0 {
  call void @dv_test()
  ret void
}

; This routine will create the dope vector, and pass it to a function
define internal void @dv_test() #0 {
  %"var$01" = alloca { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, align 8
  %"var$01_$field0$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %"var$01", i64 0, i32 0
  %"var$01_$field1$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %"var$01", i64 0, i32 1
  %"var$01_$field2$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %"var$01", i64 0, i32 2
  %"var$01_$field3$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %"var$01", i64 0, i32 3
  %"var$01_$field4$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %"var$01", i64 0, i32 4
  %"var$01_$field6$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %"var$01", i64 0, i32 6, i64 0
  %"var$01_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$01_$field6$", i64 0, i32 0
  %"var$01_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$01_$field6$", i64 0, i32 1
  %"var$01_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$01_$field6$", i64 0, i32 2
  store i64 4, ptr %"var$01_$field1$", align 8
  store i64 2, ptr %"var$01_$field4$", align 8
  store i64 0, ptr %"var$01_$field2$", align 8
  %t0 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$01_$field6$_$field1$", i32 0)
  %t1 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$01_$field6$_$field2$", i32 0)
  %t2 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$01_$field6$_$field0$", i32 0)
  store i64 4, ptr %t0, align 8
  store i64 1, ptr %t1, align 8
  store i64 9, ptr %t2, align 8
  call void @uplevel_creator(ptr %"var$01")
  ret void
}

; This routine will create the uplevel variable from an incoming dope vector
; parameter
define internal void @uplevel_creator(ptr %pDVin) #0 {
  %up = alloca %uplevel_type, align 8
  %upField = getelementptr inbounds %uplevel_type, ptr %up, i64 0, i32 0
  store ptr %pDVin, ptr %upField, align 8
  call void @uplevel_user(ptr %up)
  ret void
}

; This routine will take the uplevel variable as a parameter, and recursively
; call itself
define internal void @uplevel_user(ptr %pUplevel) #0 {
  %upField = getelementptr inbounds %uplevel_type, ptr %pUplevel, i64 0, i32 0
  %pDV = load ptr, ptr %upField, align 8
  %"var$01_$field6$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, ptr %pDV, i64 0, i32 6, i64 0
  %"var$01_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$01_$field6$", i64 0, i32 0
  %"var$01_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$01_$field6$", i64 0, i32 1
  %"var$01_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$01_$field6$", i64 0, i32 2
  %rank0.stride = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$01_$field6$_$field1$", i32 0)
  %rank0.lb = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$01_$field6$_$field2$", i32 0)
  %rank0.extent = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$01_$field6$_$field0$", i32 0)
  %stride = load i64, ptr %rank0.stride, align 4
  %lb = load i64, ptr %rank0.lb, align 4
  %extent = load i64, ptr %rank0.extent, align 4
  %check_stride = icmp eq i64 %stride, 4
; CHECK: %check_stride = icmp eq i64 4, 4
  %check_lb = icmp eq i64 %lb, 1
; CHECK: %check_lb = icmp eq i64 1, 1
  %check_extent = icmp eq i64 %extent, 9
; CHECK: %check_extent = icmp eq i64 9, 9
  br i1 undef, label %recurse, label %done

recurse:                                          ; preds = %0
  call void @uplevel_user(ptr %pUplevel)
  br label %done

done:                                             ; preds = %recurse, %0
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #1

attributes #0 = { "intel-lang"="fortran" }
attributes #1 = { nounwind readnone speculatable }
