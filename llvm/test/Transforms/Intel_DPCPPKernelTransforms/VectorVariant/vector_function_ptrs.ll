; RUN: opt %s -passes=dpcpp-kernel-sg-size-collector-indirect -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt %s -passes=dpcpp-kernel-sg-size-collector-indirect -S | FileCheck %s

%class.S1B.B = type { %class.S1A.A }
%class.S1A.A = type { i32 (...)** }
%"class.SZ4mainE3$_0.anon" = type { i8 }

$B1 = comdat any
$B2 = comdat any
$A = comdat any
$B_foo = comdat any
$V1B = comdat any
$S1B = comdat any
$I1B = comdat any

@class_type_info_B = external global i8*
@class_type_info_A = external global i8*
@V1B = constant { [3 x i8*] } { [3 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @I1B to i8*), i8* bitcast ([1 x i32 (%class.S1B.B addrspace(4)*, i32)*]* @"B_foo$SIMDTable" to i8*)] }, comdat
@S1B = constant [3 x i8] c"1B\00", comdat
@S1A = constant [3 x i8] c"1A\00"
@I1A = constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @class_type_info_A, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @S1A, i32 0, i32 0) }
@I1B = constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @class_type_info_B, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @S1B, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @I1A to i8*) }, comdat
@"B_foo$SIMDTable" = weak global [1 x i32 (%class.S1B.B addrspace(4)*, i32)*] [i32 (%class.S1B.B addrspace(4)*, i32)* @B_foo]
@V1A = constant { [3 x i8*] } { [3 x i8*] [i8* null, i8* bitcast ({ i8*, i8* }* @I1A to i8*), i8* bitcast ([1 x i32 (%class.S1A.A addrspace(4)*, i32)*]* @"A_foo$SIMDTable" to i8*)] }
@"A_foo$SIMDTable" = weak global [1 x i32 (%class.S1A.A addrspace(4)*, i32)*] [i32 (%class.S1A.A addrspace(4)*, i32)* @A_foo]

define void @test() !recommended_vector_length !0 {
entry:
  %bA = alloca %class.S1B.B
  %a = alloca %class.S1A.A addrspace(4)*
  %sum = alloca i32
  %i = alloca i32
  %0 = bitcast %class.S1B.B* %bA to i8*
  %1 = addrspacecast %class.S1B.B* %bA to %class.S1B.B addrspace(4)*
  call void @B1(%class.S1B.B addrspace(4)* %1)
  %2 = bitcast %class.S1A.A addrspace(4)** %a to i8*
  %3 = bitcast %class.S1B.B* %bA to %class.S1A.A*
  %4 = addrspacecast %class.S1A.A* %3 to %class.S1A.A addrspace(4)*
  store %class.S1A.A addrspace(4)* %4, %class.S1A.A addrspace(4)** %a
  %5 = bitcast i32* %sum to i8*
  store i32 0, i32* %sum
  %6 = bitcast i32* %i to i8*
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %7 = load i32, i32* %i
  %cmp = icmp slt i32 %7, 4
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %8 = bitcast i32* %i to i8*
  br label %for.end

for.body:                                         ; preds = %for.cond
  %9 = load %class.S1A.A addrspace(4)*, %class.S1A.A addrspace(4)** %a
  %10 = load i32, i32* %i
  %11 = bitcast %class.S1A.A addrspace(4)* %9 to i32 (%class.S1A.A addrspace(4)*, i32)** addrspace(4)*
  %vtable = load i32 (%class.S1A.A addrspace(4)*, i32)**, i32 (%class.S1A.A addrspace(4)*, i32)** addrspace(4)* %11
  %vfn = getelementptr inbounds i32 (%class.S1A.A addrspace(4)*, i32)*, i32 (%class.S1A.A addrspace(4)*, i32)** %vtable, i64 0
  %12 = load i32 (%class.S1A.A addrspace(4)*, i32)*, i32 (%class.S1A.A addrspace(4)*, i32)** %vfn
  %call = call i32 %12(%class.S1A.A addrspace(4)* %9, i32 %10)
  %13 = load i32, i32* %sum
  %add = add nsw i32 %13, %call
  store i32 %add, i32* %sum
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %14 = load i32, i32* %i
  %inc = add nsw i32 %14, 1
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  %15 = bitcast i32* %sum to i8*
  %16 = bitcast %class.S1A.A addrspace(4)** %a to i8*
  %17 = bitcast %class.S1B.B* %bA to i8*
  ret void
}

define void @B1(%class.S1B.B addrspace(4)* %this) comdat align 2 {
entry:
  %this.addr = alloca %class.S1B.B addrspace(4)*
  store %class.S1B.B addrspace(4)* %this, %class.S1B.B addrspace(4)** %this.addr
  %this1 = load %class.S1B.B addrspace(4)*, %class.S1B.B addrspace(4)** %this.addr
  call void @B2(%class.S1B.B addrspace(4)* %this1)
  ret void
}

define void @B2(%class.S1B.B addrspace(4)* %this) comdat align 2 {
entry:
  %this.addr = alloca %class.S1B.B addrspace(4)*
  store %class.S1B.B addrspace(4)* %this, %class.S1B.B addrspace(4)** %this.addr
  %this1 = load %class.S1B.B addrspace(4)*, %class.S1B.B addrspace(4)** %this.addr
  %0 = bitcast %class.S1B.B addrspace(4)* %this1 to %class.S1A.A addrspace(4)*
  call void @A(%class.S1A.A addrspace(4)* %0)
  %1 = bitcast %class.S1B.B addrspace(4)* %this1 to i32 (...)** addrspace(4)*
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [3 x i8*] }, { [3 x i8*] }* @V1B, i32 0, inrange i32 0, i32 2) to i32 (...)**), i32 (...)** addrspace(4)* %1
  ret void
}

define void @A(%class.S1A.A addrspace(4)* %this) comdat align 2 {
entry:
  %this.addr = alloca %class.S1A.A addrspace(4)*
  store %class.S1A.A addrspace(4)* %this, %class.S1A.A addrspace(4)** %this.addr
  %this1 = load %class.S1A.A addrspace(4)*, %class.S1A.A addrspace(4)** %this.addr
  %0 = bitcast %class.S1A.A addrspace(4)* %this1 to i32 (...)** addrspace(4)*
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [3 x i8*] }, { [3 x i8*] }* @V1A, i32 0, inrange i32 0, i32 2) to i32 (...)**), i32 (...)** addrspace(4)* %0
  ret void
}

define i32 @B_foo(%class.S1B.B addrspace(4)* %this, i32 %X) #4 comdat align 2 {
; CHECK: define i32 @B_foo(%class.S1B.B addrspace(4)* %this, i32 %X) #[[ATTRS1:.*]] comdat align 2 {
entry:
  %this.addr = alloca %class.S1B.B addrspace(4)*
  %X.addr = alloca i32
  store %class.S1B.B addrspace(4)* %this, %class.S1B.B addrspace(4)** %this.addr
  store i32 %X, i32* %X.addr
  %0 = load i32, i32* %X.addr
  %1 = load i32, i32* %X.addr
  %mul = mul nsw i32 %0, %1
  ret i32 %mul
}

define i32 @A_foo(%class.S1A.A addrspace(4)* %this, i32 %X) #5 align 2 {
; CHECK: define i32 @A_foo(%class.S1A.A addrspace(4)* %this, i32 %X) #[[ATTRS2:.*]] align 2 {
entry:
  %this.addr = alloca %class.S1A.A addrspace(4)*
  %X.addr = alloca i32
  store %class.S1A.A addrspace(4)* %this, %class.S1A.A addrspace(4)** %this.addr
  store i32 %X, i32* %X.addr
  %0 = load i32, i32* %X.addr
  %add = add nsw i32 %0, 1
  ret i32 %add
}

attributes #4 = { "vector_function_ptrs"="B_foo$SIMDTable()" }
attributes #5 = { "vector_function_ptrs"="A_bar1(),A_foo$SIMDTable(),A_bar2()" }

; CHECK: attributes #[[ATTRS1]] = { "vector-variants"="_ZGVbM8vv_B_foo,_ZGVbN8vv_B_foo" "vector_function_ptrs"="B_foo$SIMDTable(_ZGVbM8vv_B_foo,_ZGVbN8vv_B_foo)" }
; CHECK: attributes #[[ATTRS2]] = { "vector-variants"="_ZGVbM8vv_A_foo,_ZGVbN8vv_A_foo" "vector_function_ptrs"="A_bar1(),A_foo$SIMDTable(_ZGVbM8vv_A_foo,_ZGVbN8vv_A_foo),A_bar2()" }

!0 = !{i32 8}

; DEBUGIFY-NOT: WARNING
