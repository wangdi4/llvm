; RUN: opt -resolve-block-call -add-implicit-args -S < %s | FileCheck %s
;
; Regression test. Check if byval stuct paasing to block always have
; the same aligment on a call site and a callee.
;
; struct two_ints {
;       long x;
;       long y;
; };
; kernel void block_arg_struct()
; {
;     int (^kernelBlock)(struct two_ints) = ^int(struct two_ints ti)
;     {
;         return ti.x * ti.y;
;     };
;     struct two_ints i;
;     i.x = 2;
;     i.y = 3;
;     kernelBlock(i);
; }


; ModuleID = 'block_arg_struct.cl'
target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir"

%struct.__block_descriptor = type { i64, i64 }
%struct.two_ints = type { i64, i64 }
%struct.__block_literal_generic = type { i8*, i32, i32, i8*, %struct.__block_descriptor addrspace(2)* }

@.str = private unnamed_addr addrspace(2) constant [21 x i8] c"i20@?0{two_ints=qq}4\00", align 1
@__block_descriptor_tmp = internal addrspace(2) constant { i64, i64, i8 addrspace(2)*, i8 addrspace(2)* } { i64 0, i64 20, i8 addrspace(2)* getelementptr inbounds ([21 x i8], [21 x i8] addrspace(2)* @.str, i32 0, i32 0), i8 addrspace(2)* null }, align 4
@__block_literal_global = internal addrspace(1) constant { i8**, i32, i32, i8*, %struct.__block_descriptor addrspace(2)* } { i8** null, i32 1342177280, i32 0, i8* bitcast (i32 (i8 addrspace(4)*, %struct.two_ints*)* @__block_arg_struct_block_invoke to i8*), %struct.__block_descriptor addrspace(2)* bitcast ({ i64, i64, i8 addrspace(2)*, i8 addrspace(2)* } addrspace(2)* @__block_descriptor_tmp to %struct.__block_descriptor addrspace(2)*) }, align 4

; CHECK: define internal spir_func i32 @__block_arg_struct_block_invoke{{.*}} %struct.two_ints* [[ATTRS:byval align 8]]
define internal spir_func i32 @__block_arg_struct_block_invoke(i8 addrspace(4)* %.block_descriptor, %struct.two_ints* byval align 8 %ti) #0 {
entry:
  %x = getelementptr inbounds %struct.two_ints, %struct.two_ints* %ti, i32 0, i32 0
  %0 = load i64, i64* %x, align 8
  %y = getelementptr inbounds %struct.two_ints, %struct.two_ints* %ti, i32 0, i32 1
  %1 = load i64, i64* %y, align 8
  %mul = mul nsw i64 %0, %1
  %conv = trunc i64 %mul to i32
  ret i32 %conv
}

; Function Attrs: nounwind
define spir_kernel void @block_arg_struct() #0 {
entry:
  %kernelBlock = alloca i32 (%struct.two_ints*) addrspace(4)*, align 4
  %i = alloca %struct.two_ints, align 8
  store i32 (%struct.two_ints*) addrspace(4)* addrspacecast (i32 (%struct.two_ints*) addrspace(1)* bitcast ({ i8**, i32, i32, i8*, %struct.__block_descriptor addrspace(2)* } addrspace(1)* @__block_literal_global to i32 (%struct.two_ints*) addrspace(1)*) to i32 (%struct.two_ints*) addrspace(4)*), i32 (%struct.two_ints*) addrspace(4)** %kernelBlock, align 4
  %x = getelementptr inbounds %struct.two_ints, %struct.two_ints* %i, i32 0, i32 0
  store i64 2, i64* %x, align 8
  %y = getelementptr inbounds %struct.two_ints, %struct.two_ints* %i, i32 0, i32 1
  store i64 3, i64* %y, align 8
  %0 = load i8*, i8* addrspace(4)* getelementptr inbounds (%struct.__block_literal_generic, %struct.__block_literal_generic addrspace(4)* addrspacecast (%struct.__block_literal_generic addrspace(1)* bitcast ({ i8**, i32, i32, i8*, %struct.__block_descriptor addrspace(2)* } addrspace(1)* @__block_literal_global to %struct.__block_literal_generic addrspace(1)*) to %struct.__block_literal_generic addrspace(4)*), i32 0, i32 3), align 4
  %1 = bitcast i8* %0 to i32 (i8 addrspace(4)*, %struct.two_ints*)*
; CHECK: call spir_func i32 @__block_arg_struct_block_invoke{{.*}} %struct.two_ints* [[ATTRS]]
  %call = call spir_func i32 %1(i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast ({ i8**, i32, i32, i8*, %struct.__block_descriptor addrspace(2)* } addrspace(1)* @__block_literal_global to i8 addrspace(1)*) to i8 addrspace(4)*), %struct.two_ints* byval align 8 %i)
  ret void
}

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!llvm.ident = !{!2}

!0 = !{i32 2, i32 0}
!2 = !{!"clang version 4.0.1 "}
