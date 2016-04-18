; RUN: llvm-as %s -o %t.bc
; RUN: opt -generic-addr-static-resolution -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @func
; CHECK: %0 = bitcast i32 addrspace(1)* %add.ptr to i32 addrspace(1)*
; CHECK: %1 = addrspacecast i32 addrspace(1)* %0 to i32 addrspace(4)*
; CHECK: %arrayidx = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
; CHECK: store i32 addrspace(4)* %1, i32 addrspace(4)** %arrayidx, align 4
; CHECK: %10 = load i32 addrspace(4)** %arrayidx10, align 4
; CHECK: %11 = bitcast i32 addrspace(4)* %10 to i32 addrspace(4)*
; CHECK: %12 = load i32 addrspace(4)** %arrayidx12, align 4
; CHECK: %13 = bitcast i32 addrspace(4)* %12 to i8 addrspace(4)*
; CHECK: call i8 addrspace(1)* @_Z9to_globalPKU3AS4v(i8 addrspace(4)* %13)
; CHECK: %18 = load i32 addrspace(4)** %arrayidx26, align 4
; CHECK: %19 = bitcast i32 addrspace(4)* %18 to float addrspace(4)*
; CHECK: %add.ptr27 = getelementptr inbounds float addrspace(4)* %19, i32 10
; CHECK: %call28 = call float @_Z5fractfPU3AS4f(float %param, float addrspace(4)* %add.ptr27)
; CHECK: ret

; CHECK: !{{[0-9]+}} = !{!{{[0-9]+}}, ![[GASCount:[0-9]+]], ![[GASWarnings:[0-9]+]]}
; CHECK: ![[GASCount]] = !{!"gen_addr_space_pointer_counter", i32 0}
; CHECK: ![[GASWarnings]] = !{!"gen_addr_space_pointer_warnings"}

define void @test1(i32 addrspace(4)* %a) nounwind {
entry:
  %arrayidx = getelementptr inbounds i32 addrspace(4)* %a, i32 0
  store i32 0, i32 addrspace(4)* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds i32 addrspace(4)* %a, i32 1
  store i32 1, i32 addrspace(4)* %arrayidx1, align 4
  ret void
}

define i32 addrspace(4)* @test2(i32 addrspace(4)* %p1, i32 addrspace(4)* %p2) nounwind {
entry:
  %cmp = icmp eq i32 addrspace(4)* %p1, %p2
  br i1 %cmp, label %if.then, label %if.end

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
  %0 = addrspacecast i32 addrspace(1)* %add.ptr to i32 addrspace(4)*
  %arrayidx = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  store i32 addrspace(4)* %0, i32 addrspace(4)** %arrayidx, align 4
  br label %if.end

if.else:                                          ; preds = %for.body
  %add.ptr1 = getelementptr inbounds i32 addrspace(3)* %pLocal, i32 %i.0
  %1 = addrspacecast i32 addrspace(3)* %add.ptr1 to i32 addrspace(4)*
  %arrayidx2 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  store i32 addrspace(4)* %1, i32 addrspace(4)** %arrayidx2, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %arrayidx3 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %2 = load i32 addrspace(4)** %arrayidx3, align 4
  store i32 %i.0, i32 addrspace(4)* %2, align 4
  %arrayidx4 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %3 = load i32 addrspace(4)** %arrayidx4, align 4
  call void @test1(i32 addrspace(4)* %3)
  %arrayidx5 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %4 = load i32 addrspace(4)** %arrayidx5, align 4
  %add = add nsw i32 %i.0, 1
  %arrayidx6 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %add
  %5 = load i32 addrspace(4)** %arrayidx6, align 4
  %call = call i32 addrspace(4)* @test2(i32 addrspace(4)* %4, i32 addrspace(4)* %5)
  %arrayidx7 = getelementptr inbounds i32 addrspace(4)* %call, i32 8
  store i32 8, i32 addrspace(4)* %arrayidx7, align 4
  %arrayidx8 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %6 = load i32 addrspace(4)** %arrayidx8, align 4
  %7 = addrspacecast i32 addrspace(4)* %6 to i32 addrspace(1)*
  %arrayidx9 = getelementptr inbounds i32 addrspace(1)* %7, i32 3
  store i32 3, i32 addrspace(1)* %arrayidx9, align 4
  %arrayidx10 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %8 = load i32 addrspace(4)** %arrayidx10, align 4
  %9 = ptrtoint i32 addrspace(4)* %8 to i32
  %10 = inttoptr i32 %9 to i32 addrspace(4)*
  %arrayidx11 = getelementptr inbounds i32 addrspace(4)* %10, i32 7
  store i32 7, i32 addrspace(4)* %arrayidx11, align 4
  %arrayidx12 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %11 = load i32 addrspace(4)** %arrayidx12, align 4
  %12 = bitcast i32 addrspace(4)* %11 to i8 addrspace(4)*
  %call13 = call i8 addrspace(1)* @_Z9to_globalPKU3AS4v(i8 addrspace(4)* %12)
  %13 = bitcast i8 addrspace(1)* %call13 to i32 addrspace(1)*
  %tobool14 = icmp ne i32 addrspace(1)* %13, null
  br i1 %tobool14, label %if.then15, label %if.end17

if.then15:                                        ; preds = %if.end
  %arrayidx16 = getelementptr inbounds i32 addrspace(4)* %10, i32 9
  store i32 9, i32 addrspace(4)* %arrayidx16, align 4
  br label %if.end17

if.end17:                                         ; preds = %if.then15, %if.end
  %arrayidx18 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %14 = load i32 addrspace(4)** %arrayidx18, align 4
  %add19 = add nsw i32 %i.0, 1
  %arrayidx20 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %add19
  %15 = load i32 addrspace(4)** %arrayidx20, align 4
  %cmp21 = icmp eq i32 addrspace(4)* %14, %15
  br i1 %cmp21, label %if.then22, label %if.end25

if.then22:                                        ; preds = %if.end17
  %arrayidx23 = getelementptr inbounds i32 addrspace(4)* %10, i32 8
  %16 = load i32 addrspace(4)* %arrayidx23, align 4
  %arrayidx24 = getelementptr inbounds i32 addrspace(4)* %10, i32 10
  store i32 %16, i32 addrspace(4)* %arrayidx24, align 4
  br label %if.end25

if.end25:                                         ; preds = %if.then22, %if.end17
  %arrayidx26 = getelementptr inbounds [10 x i32 addrspace(4)*]* %ptrs, i32 0, i32 %i.0
  %17 = load i32 addrspace(4)** %arrayidx26, align 4
  %18 = bitcast i32 addrspace(4)* %17 to float addrspace(4)*
  %add.ptr27 = getelementptr inbounds float addrspace(4)* %18, i32 10
  %call28 = call float @_Z5fractfPU3AS4f(float %param, float addrspace(4)* %add.ptr27)
  br label %for.inc

for.inc:                                          ; preds = %if.end25
  %inc = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

declare i8 addrspace(1)* @_Z9to_globalPKU3AS4v(i8 addrspace(4)*)

declare float @_Z5fractfPU3AS4f(float, float addrspace(4)*)

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(3)*, float)* @func}
!1 = !{!"argument_attribute", i32 0, i32 0, i32 0}
!2 = !{!"-cl-std=CL2.0"}

;;  -----  BasicCasesArray.cl   -------
;; Command line: clang.exe -cc1 -cl-std=CL2.0 -emit-llvm -O0 -x cl -I <clang_headers> -include opencl_.h  -D__OPENCL_C_VERSION__=200 BasicCasesArray.cl -o BasicCasesArrayTmp.ll
;;               oclopt.exe -mem2reg -verify BasicCasesArrayTmp.ll -S -o BasicCasesArray.ll

;;void test1(int* a) {
;;  a[0] = 0;
;;  a[1] = 1;
;;}

;;int* test2(int *p1, int *p2) {
;;  if (p1 == p2)
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
    
;;    // Function calls
;;   test1(ptrs[i]);
;;    int* pGen5 = test2(ptrs[i], ptrs[i+1]);
;;    pGen5[8] = 8;
    
;;    // Casting
;;    __global int* pGlobal1 = (__global int*)ptrs[i];
;;    pGlobal1[3] = 3;
    
;;    // Conversion to/from integer
;;  	size_t intGen2 = (size_t)ptrs[i];
;;  	int* pGen3 = (int*)intGen2;
;;  	pGen3[7] = 7;
  	
;;  	// Address Specifier BI
;;   	__global int* b2 = to_global(ptrs[i]);
;;  	if (b2) {
;;    	pGen3[9] = 9;
;;    }

;;	  // ICMP, load
;;	  if (ptrs[i] == ptrs[i+1]) {
;;	   pGen3[10] = pGen3[8];
;;	  }
;;	  // BI with generic addr space
;;	  float* pGen4 = (float*)ptrs[i];
;;	  float res = fract(param, pGen4 + 10);   
;;  }
;;}
