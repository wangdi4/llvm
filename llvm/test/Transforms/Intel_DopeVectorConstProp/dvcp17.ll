; RUN: opt < %s -dope-vector-local-const-prop=false -S -passes=dopevectorconstprop -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

; Test dope vector analysis and constant propagation when uplevel
; variables are passed to a recursive function.

; This is the same test as dvcp07.ll, but checks the IR rather than the traces.

; Test case uses 1 rank dope vector which looks like:
; { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

; Uplevel variable will consist of a single dope vector
%uplevel_type = type { { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* }

define dso_local void @MAIN__() #0 {
  call void @dv_test();
  ret void
}

; This routine will create the dope vector, and pass it to a function
define internal void @dv_test() #0 {
  %"var$01" = alloca { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, align 8
  %"var$01_$field0$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 0
  %"var$01_$field1$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 1
  %"var$01_$field2$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 2
  %"var$01_$field3$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 3
  %"var$01_$field4$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 4
  %"var$01_$field6$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 6, i64 0
  %"var$01_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$01_$field6$", i64 0, i32 0
  %"var$01_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$01_$field6$", i64 0, i32 1
  %"var$01_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$01_$field6$", i64 0, i32 2
  store i64 4, i64* %"var$01_$field1$", align 8
  store i64 2, i64* %"var$01_$field4$", align 8
  store i64 0, i64* %"var$01_$field2$", align 8

  ; Populate rank 0 fields
  %t0 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$01_$field6$_$field1$", i32 0)
  %t1 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$01_$field6$_$field2$", i32 0)
  %t2 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$01_$field6$_$field0$", i32 0)

  ; Store stride, lower bound and extent
  store i64 4, i64* %t0, align 8
  store i64 1, i64* %t1, align 8
  store i64 9, i64* %t2, align 8

  call void @uplevel_creator({ i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* %"var$01")
  ret void
}

; This routine will create the uplevel variable from an incoming dope vector
; parameter
define internal void @uplevel_creator({ i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* %pDVin) #0 {
  %up = alloca %uplevel_type
  %upField = getelementptr inbounds %uplevel_type, %uplevel_type* %up, i64 0, i32 0
  store { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* %pDVin, { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }** %upField, align 8
  call void @uplevel_user(%uplevel_type *%up)
  ret void
}

; This routine will take the uplevel variable as a parameter, and recursively
; call itself
define internal void @uplevel_user(%uplevel_type* %pUplevel) #0 {
  %upField = getelementptr inbounds %uplevel_type, %uplevel_type* %pUplevel, i64 0, i32 0
  %pDV = load { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }*, { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }** %upField

  %"var$01_$field6$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }* %pDV, i64 0, i32 6, i64 0
  %"var$01_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$01_$field6$", i64 0, i32 0
  %"var$01_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$01_$field6$", i64 0, i32 1
  %"var$01_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$01_$field6$", i64 0, i32 2

  ; Load stride, lower bound, and extent
  %rank0.stride = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$01_$field6$_$field1$", i32 0)
  %rank0.lb = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$01_$field6$_$field2$", i32 0)
  %rank0.extent = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"var$01_$field6$_$field0$", i32 0)
  %stride = load i64, i64* %rank0.stride
  %lb = load i64, i64* %rank0.lb
  %extent = load i64, i64* %rank0.extent

 ; These uses of the values from the dope vector can be replaced with constant
 ; propagated values.
 %check_stride = icmp eq i64 %stride, 4
; CHECK: %check_stride = icmp eq i64 4, 4
 %check_lb = icmp eq i64 %lb, 1
; CHECK: %check_lb = icmp eq i64 1, 1
 %check_extent = icmp eq i64 %extent, 9
; CHECK: %check_extent = icmp eq i64 9, 9

  br i1 undef, label %recurse, label %done
recurse:
  call void @uplevel_user(%uplevel_type* %pUplevel)
  br label %done

done:
  ret void
}

declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32)

attributes #0 = {"intel-lang"="fortran"}
