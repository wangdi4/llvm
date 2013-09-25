; RUN: llvm-as %s -o %t.bc
; RUN: opt -generic-addr-dynamic-resolution -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK-NOT: @test1
; CHECK-NOT: @test2

; CHECK:  @func
; CHECK:  %AllocaSpace20 = alloca i32
; CHECK:  %ptrs = alloca [10 x i32 addrspace(4)*], align 4
; CHECK:  %AllocaSpace = mul i32 10, 1
; CHECK:  %AllocaSpace1 = alloca i32, i32 %AllocaSpace
; CHECK:  %AllocaSpace2 = ptrtoint [10 x i32 addrspace(4)*]* %ptrs to i64
; CHECK:  %AllocaSpace3 = ptrtoint i32* %AllocaSpace1 to i64
; CHECK:  %AllocaSpace4 = sub i64 %AllocaSpace3, %AllocaSpace2
; CHECK:  store i32 addrspace(4)* %0, i32 addrspace(4)** %arrayidx, align 4
; CHECK:  %AllocaSpace41 = ptrtoint i32 addrspace(4)** %arrayidx to i64
; CHECK:  %AllocaSpace42 = add i64 %AllocaSpace41, %AllocaSpace4
; CHECK:  %AllocaSpace43 = inttoptr i64 %AllocaSpace42 to i32*
; CHECK:  store i32 1, i32* %AllocaSpace43
; CHECK:  store i32 addrspace(4)* %1, i32 addrspace(4)** %arrayidx2, align 4
; CHECK:  %AllocaSpace38 = ptrtoint i32 addrspace(4)** %arrayidx2 to i64
; CHECK:  %AllocaSpace39 = add i64 %AllocaSpace38, %AllocaSpace4
; CHECK:  %AllocaSpace40 = inttoptr i64 %AllocaSpace39 to i32*
; CHECK:  store i32 3, i32* %AllocaSpace40
; CHECK:  %3 = load i32 addrspace(4)** %arrayidx4, align 4
; CHECK:  %AllocaSpace34 = ptrtoint i32 addrspace(4)** %arrayidx4 to i64
; CHECK:  %AllocaSpace35 = add i64 %AllocaSpace34, %AllocaSpace4
; CHECK:  %AllocaSpace36 = inttoptr i64 %AllocaSpace35 to i32*
; CHECK:  %AddrSpace37 = load i32* %AllocaSpace36
; CHECK-NOT: %call = call zeroext i1 @_Z9is_globalPKU3AS4v(i8 addrspace(4)* %4)
; CHECK:  %AddrSpace44 = icmp eq i32 %AddrSpace37, 1
; CHECK:  %5 = load i32 addrspace(4)** %arrayidx5, align 4
; CHECK:  %AllocaSpace30 = ptrtoint i32 addrspace(4)** %arrayidx5 to i64
; CHECK:  %AllocaSpace31 = add i64 %AllocaSpace30, %AllocaSpace4
; CHECK:  %AllocaSpace32 = inttoptr i64 %AllocaSpace31 to i32*
; CHECK:  %AddrSpace33 = load i32* %AllocaSpace32
; CHECK-NOT: call void @test1(i32 addrspace(4)* %5)   
; CHECK:	call void @_Z5test1PU3AS4i(i32 addrspace(4)* %5, i32 %AddrSpace33)
; CHECK:	%6 = load i32 addrspace(4)** %arrayidx6, align 4
; CHECK:  %AllocaSpace26 = ptrtoint i32 addrspace(4)** %arrayidx6 to i64
; CHECK:  %AllocaSpace27 = add i64 %AllocaSpace26, %AllocaSpace4
; CHECK:  %AllocaSpace28 = inttoptr i64 %AllocaSpace27 to i32*
; CHECK:  %AddrSpace29 = load i32* %AllocaSpace28
; CHECK:	%7 = load i32 addrspace(4)** %arrayidx7, align 4
; CHECK:  %AllocaSpace22 = ptrtoint i32 addrspace(4)** %arrayidx7 to i64
; CHECK:  %AllocaSpace23 = add i64 %AllocaSpace22, %AllocaSpace4
; CHECK:  %AllocaSpace24 = inttoptr i64 %AllocaSpace23 to i32*
; CHECK:  %AddrSpace25 = load i32* %AllocaSpace24
; CHECK-NOT: %call8 = call i32 addrspace(4)* @test2(i32 addrspace(4)* %6, i32 addrspace(4)* %7)
; CHECK:	%8 = call i32 addrspace(4)* @_Z5test2PU3AS4iS_PU3AS0i(i32 addrspace(4)* %6, i32 addrspace(4)* %7, i32 %AddrSpace29, i32 %AddrSpace25, i32* %AllocaSpace20)
; CHECK:  %AddrSpace21 = load i32* %AllocaSpace20
; CHECK-NOT: %call9 = call zeroext i1 @_Z10is_privatePKU3AS4v(i8 addrspace(4)* %8)
; CHECK:  %AddrSpace45 = icmp eq i32 %AddrSpace21, 0   	
; CHECK-NOT: %call15 = call i32 @get_fence(i8 addrspace(4)* %15)
; CHECK:  %AddrSpace47 = icmp eq i32 %AddrSpace19, 1
; CHECK:  %AddrSpace48 = icmp eq i32 %AddrSpace19, 3
; CHECK:  %AddrSpace49 = select i1 %AddrSpace47, i32 2, i32 0
; CHECK:  %AddrSpace50 = select i1 %AddrSpace48, i32 1, i32 %AddrSpace49
; CHECK:  %arrayidx25 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
; CHECK:  %21 = load i32 addrspace(4)** %arrayidx25, align 4
; CHECK:  %add26 = add nsw i32 %i.0, 1
; CHECK:  %arrayidx27 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %add26
; CHECK:  %22 = load i32 addrspace(4)** %arrayidx27, align 4
; CHECK:  %cmp28 = icmp eq i32 addrspace(4)* %21, %22
; CHECK-NOT: %call36 = call float @_Z5fractfPU3AS4f(float %param, float addrspace(4)* %add.ptr35)
; CHECK:  %AddrSpace54 = bitcast float addrspace(4)* %add.ptr35 to float addrspace(1)*
; CHECK:  %27 = call float @_Z5fractfPU3AS1f(float %param, float addrspace(1)* %AddrSpace54)
; CHECK:  ret
  
; CHECK:	define i32 addrspace(4)* @_Z5test2PU3AS4iS_PU3AS0i(i32 addrspace(4)* %p1, i32 addrspace(4)* %p2, i32 %ArgSpace, i32 %ArgSpace1, i32* %ArgSpace3)
; CHECK:  %0 = bitcast i32 addrspace(4)* %p1 to i8 addrspace(4)*
; CHECK:  %AddrSpace2 = icmp eq i32 %ArgSpace, 1
; CHECK:  %retval.0 = phi i32 addrspace(4)* [ %p1, %if.then ], [ %p2, %if.end ]
; CHECK:  %AddrSpace = phi i32 [ %ArgSpace, %if.then ], [ %ArgSpace1, %if.end ]
; CHECK:  store i32 %AddrSpace, i32* %ArgSpace3
; CHECK:  ret i32 addrspace(4)* %retval.0

; CHECK:	declare float @_Z5fractfPU3AS1f(float, float addrspace(1)*)	
	
; ModuleID = 'BasicCasesDynamicArrayTmp.ll'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @test1(i32 addrspace(4)* %p) nounwind {
entry:
  %0 = bitcast i32 addrspace(4)* %p to i8 addrspace(4)*
  %call = call zeroext i1 @_Z8is_localPKU3AS4v(i8 addrspace(4)* %0)
  %frombool = zext i1 %call to i8
  %1 = bitcast i32 addrspace(4)* %p to i8 addrspace(4)*
  %call1 = call zeroext i1 @_Z9is_globalPKU3AS4v(i8 addrspace(4)* %1)
  %frombool2 = zext i1 %call1 to i8
  ret void
}

declare zeroext i1 @_Z8is_localPKU3AS4v(i8 addrspace(4)*)

declare zeroext i1 @_Z9is_globalPKU3AS4v(i8 addrspace(4)*)

define i32 addrspace(4)* @test2(i32 addrspace(4)* %p1, i32 addrspace(4)* %p2) nounwind {
entry:
  %0 = bitcast i32 addrspace(4)* %p1 to i8 addrspace(4)*
  %call = call zeroext i1 @_Z9is_globalPKU3AS4v(i8 addrspace(4)* %0)
  br i1 %call, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %return

if.end:                                           ; preds = %entry
  br label %return

return:                                           ; preds = %if.end, %if.then
  %retval.0 = phi i32 addrspace(4)* [ %p1, %if.then ], [ %p2, %if.end ]
  ret i32 addrspace(4)* %retval.0
}

define void @func(i32 addrspace(1)* %pGlobal, i32 addrspace(3)* %pLocal, float %param) nounwind {
entry:
  %ptrs = alloca [10 x i32 addrspace(4)*], align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp slt i32 %i.0, 10
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %rem = srem i32 %i.0, 2
  %tobool = icmp ne i32 %rem, 0
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %add.ptr = getelementptr inbounds i32 addrspace(1)* %pGlobal, i32 %i.0
  %0 = bitcast i32 addrspace(1)* %add.ptr to i32 addrspace(4)*
  %arrayidx = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  store i32 addrspace(4)* %0, i32 addrspace(4)** %arrayidx, align 4
  br label %if.end

if.else:                                          ; preds = %for.body
  %add.ptr1 = getelementptr inbounds i32 addrspace(3)* %pLocal, i32 %i.0
  %1 = bitcast i32 addrspace(3)* %add.ptr1 to i32 addrspace(4)*
  %arrayidx2 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  store i32 addrspace(4)* %1, i32 addrspace(4)** %arrayidx2, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %arrayidx3 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %2 = load i32 addrspace(4)** %arrayidx3, align 4
  store i32 %i.0, i32 addrspace(4)* %2, align 4
  %arrayidx4 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %3 = load i32 addrspace(4)** %arrayidx4, align 4
  %4 = bitcast i32 addrspace(4)* %3 to i8 addrspace(4)*
  %call = call zeroext i1 @_Z9is_globalPKU3AS4v(i8 addrspace(4)* %4)
  %frombool = zext i1 %call to i8
  %arrayidx5 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %5 = load i32 addrspace(4)** %arrayidx5, align 4
  call void @test1(i32 addrspace(4)* %5)
  %arrayidx6 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %6 = load i32 addrspace(4)** %arrayidx6, align 4
  %add = add nsw i32 %i.0, 1
  %arrayidx7 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %add
  %7 = load i32 addrspace(4)** %arrayidx7, align 4
  %call8 = call i32 addrspace(4)* @test2(i32 addrspace(4)* %6, i32 addrspace(4)* %7)
  %8 = bitcast i32 addrspace(4)* %call8 to i8 addrspace(4)*
  %call9 = call zeroext i1 @_Z10is_privatePKU3AS4v(i8 addrspace(4)* %8)
  %frombool10 = zext i1 %call9 to i8
  %arrayidx11 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %9 = load i32 addrspace(4)** %arrayidx11, align 4
  %10 = bitcast i32 addrspace(4)* %9 to i32 addrspace(1)*
  %11 = bitcast i32 addrspace(1)* %10 to i8 addrspace(4)*
  %call12 = call zeroext i1 @_Z10is_privatePKU3AS4v(i8 addrspace(4)* %11)
  %frombool13 = zext i1 %call12 to i8
  %arrayidx14 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %12 = load i32 addrspace(4)** %arrayidx14, align 4
  %13 = ptrtoint i32 addrspace(4)* %12 to i32
  %14 = inttoptr i32 %13 to i32 addrspace(4)*
  %15 = bitcast i32 addrspace(4)* %14 to i8 addrspace(4)*
  %call15 = call i32 @get_fence(i8 addrspace(4)* %15)
  %arrayidx16 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %16 = load i32 addrspace(4)** %arrayidx16, align 4
  %17 = bitcast i32 addrspace(4)* %16 to i8 addrspace(4)*
  %call17 = call zeroext i1 @_Z9is_globalPKU3AS4v(i8 addrspace(4)* %17)
  %frombool18 = zext i1 %call17 to i8
  %tobool19 = trunc i8 %frombool18 to i1
  br i1 %tobool19, label %if.then20, label %if.end24

if.then20:                                        ; preds = %if.end
  %arrayidx21 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 9
  %18 = load i32 addrspace(4)** %arrayidx21, align 4
  %19 = bitcast i32 addrspace(4)* %18 to i8 addrspace(4)*
  %call22 = call zeroext i1 @_Z9is_globalPKU3AS4v(i8 addrspace(4)* %19)
  %frombool23 = zext i1 %call22 to i8
  br label %if.end24

if.end24:                                         ; preds = %if.then20, %if.end
  %arrayidx25 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %20 = load i32 addrspace(4)** %arrayidx25, align 4
  %add26 = add nsw i32 %i.0, 1
  %arrayidx27 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %add26
  %21 = load i32 addrspace(4)** %arrayidx27, align 4
  %cmp28 = icmp eq i32 addrspace(4)* %20, %21
  br i1 %cmp28, label %if.then29, label %if.end33

if.then29:                                        ; preds = %if.end24
  %arrayidx30 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %22 = load i32 addrspace(4)** %arrayidx30, align 4
  %23 = bitcast i32 addrspace(4)* %22 to i8 addrspace(4)*
  %call31 = call zeroext i1 @_Z10is_privatePKU3AS4v(i8 addrspace(4)* %23)
  %frombool32 = zext i1 %call31 to i8
  br label %if.end33

if.end33:                                         ; preds = %if.then29, %if.end24
  %arrayidx34 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %24 = load i32 addrspace(4)** %arrayidx34, align 4
  %25 = bitcast i32 addrspace(4)* %24 to float addrspace(4)*
  %add.ptr35 = getelementptr inbounds float addrspace(4)* %25, i32 10
  %call36 = call float @_Z5fractfPU3AS4f(float %param, float addrspace(4)* %add.ptr35)
  br label %for.inc

for.inc:                                          ; preds = %if.end33
  %inc = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

declare zeroext i1 @_Z10is_privatePKU3AS4v(i8 addrspace(4)*)

declare i32 @get_fence(i8 addrspace(4)*)

declare float @_Z5fractfPU3AS4f(float, float addrspace(4)*)

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}

!0 = metadata !{void (i32 addrspace(1)*, i32 addrspace(3)*, float)* @func}

;;  -----  BasicCasesDynamicArray.cl   -------
;; Command line: clang.exe -cc1 -cl-std=CL2.0 -emit-llvm -O0 -x cl -I <clang_headers> -include opencl_.h  -D__OPENCL_C_VERSION__=200 BasicCasesDynamicArray.cl -o BasicCasesDynamicArrayTmp.ll
;;               oclopt.exe -mem2reg -verify BasicCasesDynamicArrayTmp.ll -S -o BasicCasesDynamicArray.ll

;;void test1(int* p) {
;;  bool a = is_local(p);
;;  bool b = is_global(p);
;;}


;;int* test2(int *p1, int *p2) {
;;  if (is_global(p1))
;;    return p1;
;;  return p2;
;;}

;;__kernel void func(__global int *pGlobal, __local int *pLocal, float param) {
;;  int* ptrs[10];
;;  for (int i = 0; i < 10; i++) {
  
;;    // Initialization
;;    if (i%2) {
;;       ptrs[i] = pGlobal + i;
;;    } else {
;;       ptrs[i] = pLocal + i;
;;    }
;;    *ptrs[i] = i;
;;    bool a = is_global(ptrs[i]);
    
;;    // Function calls
;;    test1(ptrs[i]);
;;    int* pGen5 = test2(ptrs[i], ptrs[i+1]);
;;    bool d = is_private(pGen5);
    
;;    // Casting
;;    __global int* pGlobal1 = (__global int*)ptrs[i];
;;    bool c = is_private(pGlobal1);
    
;;    // Conversion to/from integer
;;  	size_t intGen2 = (size_t)ptrs[i];
;;  	int* pGen3 = (int*)intGen2;
;;  	cl_mem_fence_flags foo = get_fence(pGen3);
  	
;;  	// Address Specifier BI
;;   	bool b2 = is_global(ptrs[i]);
;;  	if (b2) {
;;    	bool f = is_global(ptrs[9]);
;;    }

;;	  // ICMP, load
;;	  if (ptrs[i] == ptrs[i+1]) {
;;	   bool e = is_private(ptrs[i]);
;;	  }

;;	  // BI with generic addr space
;;	  float* pGen4 = (float*)ptrs[i];
;;	  float res = fract(param, pGen4 + 10);   
;;  }
;;}
