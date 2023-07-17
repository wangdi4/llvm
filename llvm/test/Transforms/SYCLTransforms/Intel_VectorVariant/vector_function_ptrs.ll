; RUN: opt %s -passes=sycl-kernel-sg-size-collector-indirect -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt %s -passes=sycl-kernel-sg-size-collector-indirect -S | FileCheck %s

%class.S1B.B = type { %class.S1A.A }
%class.S1A.A = type { ptr }
%"class.SZ4mainE3$_0.anon" = type { i8 }

$B1 = comdat any
$B2 = comdat any
$A = comdat any
$B_foo = comdat any
$V1B = comdat any
$S1B = comdat any
$I1B = comdat any

@class_type_info_B = external global ptr
@class_type_info_A = external global ptr
@V1B = constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr @I1B, ptr @"B_foo$SIMDTable"] }, comdat
@S1B = constant [3 x i8] c"1B\00", comdat
@S1A = constant [3 x i8] c"1A\00"
@I1A = constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @class_type_info_A, i64 2), ptr @S1A }
@I1B = constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @class_type_info_B, i64 2), ptr @S1B, ptr @I1A }, comdat
@"B_foo$SIMDTable" = weak global [1 x ptr] [ptr @B_foo]
@V1A = constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr @I1A, ptr @"A_foo$SIMDTable"] }
@"A_foo$SIMDTable" = weak global [1 x ptr] [ptr @A_foo]

define void @test() !recommended_vector_length !0 {
entry:
  %bA = alloca %class.S1B.B
  %a = alloca ptr addrspace(4)
  %sum = alloca i32
  %i = alloca i32
  %0 = addrspacecast ptr %bA to ptr addrspace(4)
  call void @B1(ptr addrspace(4) %0)
  %1 = addrspacecast ptr %bA to ptr addrspace(4)
  store ptr addrspace(4) %1, ptr %a
  store i32 0, ptr %sum
  store i32 0, ptr %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %2 = load i32, ptr %i
  %cmp = icmp slt i32 %2, 4
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  br label %for.end

for.body:                                         ; preds = %for.cond
  %3 = load ptr addrspace(4), ptr %a
  %4 = load i32, ptr %i
  %vtable = load ptr, ptr addrspace(4) %3
  %5 = load ptr, ptr %vtable
  %call = call i32 %5(ptr addrspace(4) %3, i32 %4)
  %6 = load i32, ptr %sum
  %add = add nsw i32 %6, %call
  store i32 %add, ptr %sum
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %7 = load i32, ptr %i
  %inc = add nsw i32 %7, 1
  store i32 %inc, ptr %i
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  ret void
}

define void @B1(ptr addrspace(4) %this) comdat align 2 !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %this.addr = alloca ptr addrspace(4)
  store ptr addrspace(4) %this, ptr %this.addr
  %this1 = load ptr addrspace(4), ptr %this.addr
  call void @B2(ptr addrspace(4) %this1)
  ret void
}

define void @B2(ptr addrspace(4) %this) comdat align 2 !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %this.addr = alloca ptr addrspace(4)
  store ptr addrspace(4) %this, ptr %this.addr
  %this1 = load ptr addrspace(4), ptr %this.addr
  call void @A(ptr addrspace(4) %this1)
  store ptr getelementptr inbounds ({ [3 x ptr] }, ptr @V1B, i32 0, inrange i32 0, i32 2), ptr addrspace(4) %this1
  ret void
}

define void @A(ptr addrspace(4) %this) comdat align 2 !kernel_arg_base_type !3 !arg_type_null_val !4 {
entry:
  %this.addr = alloca ptr addrspace(4)
  store ptr addrspace(4) %this, ptr %this.addr
  %this1 = load ptr addrspace(4), ptr %this.addr
  store ptr getelementptr inbounds ({ [3 x ptr] }, ptr @V1A, i32 0, inrange i32 0, i32 2), ptr addrspace(4) %this1
  ret void
}

define i32 @B_foo(ptr addrspace(4) %this, i32 %X) #4 comdat align 2 !kernel_arg_base_type !5 !arg_type_null_val !6 {
; CHECK: define i32 @B_foo(ptr addrspace(4) %this, i32 %X) #[[ATTRS1:.*]] comdat align 2
entry:
  %this.addr = alloca ptr addrspace(4)
  %X.addr = alloca i32
  store ptr addrspace(4) %this, ptr %this.addr
  store i32 %X, ptr %X.addr
  %0 = load i32, ptr %X.addr
  %1 = load i32, ptr %X.addr
  %mul = mul nsw i32 %0, %1
  ret i32 %mul
}

define i32 @A_foo(ptr addrspace(4) %this, i32 %X) #5 align 2 !kernel_arg_base_type !7 !arg_type_null_val !8 {
; CHECK: define i32 @A_foo(ptr addrspace(4) %this, i32 %X) #[[ATTRS2:.*]] align 2
entry:
  %this.addr = alloca ptr addrspace(4)
  %X.addr = alloca i32
  store ptr addrspace(4) %this, ptr %this.addr
  store i32 %X, ptr %X.addr
  %0 = load i32, ptr %X.addr
  %add = add nsw i32 %0, 1
  ret i32 %add
}

attributes #4 = { "vector_function_ptrs"="B_foo$SIMDTable()" }
attributes #5 = { "vector_function_ptrs"="A_bar1(),A_foo$SIMDTable(),A_bar2()" }

; CHECK: attributes #[[ATTRS1]] = { "vector-variants"="_ZGVbM8vv_B_foo,_ZGVbN8vv_B_foo" "vector_function_ptrs"="B_foo$SIMDTable(_ZGVbM8vv_B_foo,_ZGVbN8vv_B_foo)" }
; CHECK: attributes #[[ATTRS2]] = { "vector-variants"="_ZGVbM8vv_A_foo,_ZGVbN8vv_A_foo" "vector_function_ptrs"="A_bar1(),A_foo$SIMDTable(_ZGVbM8vv_A_foo,_ZGVbN8vv_A_foo),A_bar2()" }

!0 = !{i32 8}
!1 = !{!"%class.S1B.B*"}
!2 = !{ptr addrspace(4) null}
!3 = !{!"%class.S1A.A*"}
!4 = !{ptr addrspace(4) null}
!5 = !{!"%class.S1B.B*", !"int"}
!6 = !{ptr addrspace(4) null, i32 0}
!7 = !{!"%class.S1A.A*", !"int"}
!8 = !{ptr addrspace(4) null, i32 0}

; DEBUGIFY-NOT: WARNING
