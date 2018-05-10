; This test checks that the translator doesn't crash if the module has literal
; structs, i.e. structs whose type has no name. Typicaly clang generate such
; structs if the kernel contains OpenCL 2.0 blocks. The IR was produced with
; the following command:
; clang -cc1 -triple spir -cl-std=cl2.0 -finclude-default-header literl-struct.cl -emit-llvm -o  test/SPIRV/literal-struct2.ll

; Since we are checking only literal struct translation and don't want this test
; to fail if the resolving opencl block calls functionality is broken,
; initializer of __block_literal_global is simplified and body of function 'foo'
; is commented out.

; literal-struct.cl:
; void foo()
; {
;   void (^myBlock)(void) = ^{};
;   myBlock();
; }

; RUN: llvm-as < %s | llvm-spirv -spirv-text -o %t
; RUN: FileCheck < %t %s

; CHECK-DAG: TypeInt [[Int:[0-9]+]] 32 0
; CHECK-DAG: TypePointer [[Ptr:[0-9]+]]
; CHECK-DAG: TypeStruct [[StructType:[0-9]+]] [[Int]] [[Int]] [[Ptr]]

target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir"

@__block_literal_global = internal addrspace(1) constant { i32, i32, i8 addrspace(4)* } { i32 12, i32 4, i8 addrspace(4)* null }, align 4
; CHECK: ConstantComposite [[StructType]]

; This is artificial case to cover code which produces SPIRV ConstantNull instrucitions with TypeStruct.
@__block_literal_global.1 = internal addrspace(1) constant { i32, i32, i8 addrspace(4)* } zeroinitializer, align 4
; CHECK: ConstantNull [[StructType]]

; Function Attrs: convergent nounwind
define spir_func void @foo() local_unnamed_addr #0 {
entry:
;  %0 = load void (i8 addrspace(4)*) addrspace(4)*, void (i8 addrspace(4)*) addrspace(4)* addrspace(4)* bitcast (i8 addrspace(4)* addrspace(4)* getelementptr inbounds ({ i32, i32, i8 addrspace(4)* }, { i32, i32, i8 addrspace(4)* } addrspace(4)* addrspacecast ({ i32, i32, i8 addrspace(4)* } addrspace(1)* @__block_literal_global to { i32, i32, i8 addrspace(4)* } addrspace(4)*), i32 0, i32 2) to void (i8 addrspace(4)*) addrspace(4)* addrspace(4)*), align 4
;  %1 = addrspacecast void (i8 addrspace(4)*) addrspace(4)* %0 to void (i8 addrspace(4)*)*
;  tail call spir_func void %1(i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast ({ i32, i32, i8 addrspace(4)* } addrspace(1)* @__block_literal_global to i8 addrspace(1)*) to i8 addrspace(4)*)) #2
  ret void
}

; Function Attrs: norecurse nounwind readnone
define internal spir_func void @__foo_block_invoke(i8 addrspace(4)* nocapture %.block_descriptor) #1 {
entry:
  ret void
}

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { norecurse nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent nounwind }

!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 2, i32 0}
!2 = !{}
!3 = !{!"clang version 6.0.0 (cfe/trunk)"}
