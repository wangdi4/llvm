; RUN: %oclopt -add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -add-implicit-args %s -S | FileCheck %s
;
; Generated from:
; typedef int (*fp_t)(int);
;
; __kernel void test(__global int *fp, __global int *data) {
;
;   data[0] = ((fp_t)fp)(data[1]);
; }
;
; CHECK: define spir_func i32 @foo(i32 %arg, i8 addrspace(3)* noalias %pLocalMemBase,
; CHECK-SAME:          { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* noalias %pWorkDim
; CHECK-SAME:          i64* noalias %pWGId,
; CHECK-SAME:          [4 x i64] %BaseGlbId,
; CHECK-SAME:          i8* noalias %pSpecialBuf,
; CHECK-SAME:          {}* noalias %RuntimeHandle)
;
; CHECK: define spir_kernel void @test
; CHECK: %[[BITCAST:[0-9]+]] = bitcast i32 (i32)* %fp to i32 (i32, i8 addrspace(3)*, { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }*, i64*, [4 x i64], i8*, {}*)*
; CHECK: call i32 %[[BITCAST]](i32 %0,

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

; Function Attrs: convergent noinline nounwind optnone
define spir_func i32 @foo(i32 %arg) #2 {
entry:
  %arg.addr = alloca i32, align 4
  store i32 %arg, i32* %arg.addr, align 4
  %0 = load i32, i32* %arg.addr, align 4
  %add = add nsw i32 %0, 10
  ret i32 %add
}

; Function Attrs: convergent nounwind
define spir_kernel void @test(i32 (i32)* %fp, i32 addrspace(1)* %data) #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !5 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !7 {
entry:
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %data, i64 1
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 4, !tbaa !8
  %call = call spir_func i32 %fp(i32 %0) #1
  %arrayidx1 = getelementptr inbounds i32, i32 addrspace(1)* %data, i64 0
  store i32 %call, i32 addrspace(1)* %arrayidx1, align 4, !tbaa !8
  ret void
}

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent }
attributes #2 = { convergent noinline nounwind optnone }

!llvm.module.flags = !{!0}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 0}
!2 = !{i32 1, i32 2}
!3 = !{!"clang version 7.1.0 "}
!4 = !{i32 1, i32 1}
!5 = !{!"none", !"none"}
!6 = !{!"int*", !"int*"}
!7 = !{!"", !""}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} getelementptr
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test {{.*}} bitcast
; DEBUGIFY-NOT: WARNING
