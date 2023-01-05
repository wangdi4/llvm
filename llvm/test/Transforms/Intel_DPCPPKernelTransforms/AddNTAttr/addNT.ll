; RUN: opt -passes=dpcpp-kernel-add-nt-attr -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-add-nt-attr -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK-LABEL: @test0
; CHECK-LABEL: entry
define void @test0(i32 addrspace(1)* %src, i32 addrspace(1)* %dst) local_unnamed_addr #0 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %srcidx = getelementptr inbounds i32, i32 addrspace(1)* %src, i64 %call
  %val = load i32, i32 addrspace(1)* %srcidx, align 4
  %dstidx = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %call
  ; CHECK-NOT: !nontemporal
  store i32 %val, i32 addrspace(1)* %dstidx, align 4
  ret void
}

; CHECK-LABEL: @test1
; CHECK-LABEL: entry
define void @test1(i32 addrspace(1)* noalias %src, i32 addrspace(1)* noalias %dst) local_unnamed_addr #0 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %srcidx = getelementptr inbounds i32, i32 addrspace(1)* %src, i64 %call
  %val = load i32, i32 addrspace(1)* %srcidx, align 4
  %dstidx = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %call
  ; CHECK: store i32 %val, i32 addrspace(1)* %dstidx, align 4, !nontemporal !3
  store i32 %val, i32 addrspace(1)* %dstidx, align 4
  ret void
}

; CHECK-LABEL: @test2
; CHECK-LABEL: entry
define void @test2(i32 addrspace(1)* noalias %src, i32 addrspace(1)* noalias %dst) local_unnamed_addr #0 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %srcidx = getelementptr inbounds i32, i32 addrspace(1)* %src, i64 %call
  %val = load i32, i32 addrspace(1)* %srcidx, align 4
  %dstidx = getelementptr inbounds i32, i32 addrspace(1)* %src, i32 4
  ; CHECK-NOT: !nontemporal
  store i32 %val, i32 addrspace(1)* %dstidx, align 4
  ret void
}


; CHECK-LABEL: @test3
; CHECK-LABEL: entry
define void @test3(i32 addrspace(1)* noalias %src, i32 addrspace(1)* noalias %dst) local_unnamed_addr #0 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %srcidx = getelementptr inbounds i32, i32 addrspace(1)* %src, i64 %call
  %val = load i32, i32 addrspace(1)* %srcidx, align 4
  %dstidx = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %call
  ; CHECK-NOT: !nontemporal
  store i32 %val, i32 addrspace(1)* %dstidx, align 4
  %val2 = load i32, i32 addrspace(1)* %dstidx, align 4
  ret void
}

; CHECK-LABEL: @test4
; CHECK-LABEL: entry
define void @test4(i32 addrspace(1)* noalias %src, i32 addrspace(1)* noalias %dst) local_unnamed_addr #0 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %srcidx = getelementptr inbounds i32, i32 addrspace(1)* %src, i64 %call
  %val = load i32, i32 addrspace(1)* %srcidx, align 4
  %dstidx = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %call
  ; CHECK-NOT: !nontemporal
  store i32 %val, i32 addrspace(1)* %dstidx, align 4
  br label %next
next:
  %val2 = load i32, i32 addrspace(1)* %dstidx, align 4
  br label %exit
exit:
  ret void
}

; CHECK-LABEL: @test5
; CHECK-LABEL: entry
define void @test5(i32 addrspace(1)* noalias %src, i32 addrspace(1)* noalias %dst) local_unnamed_addr #0 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %srcidx = getelementptr inbounds i32, i32 addrspace(1)* %src, i64 %call
  %val = load i32, i32 addrspace(1)* %srcidx, align 4
  %dstidx = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %call
  ; CHECK: store i32 %val, i32 addrspace(1)* %dstidx, align 4, !nontemporal !3
  store i32 %val, i32 addrspace(1)* %dstidx, align 4
  br label %next
next:
  %val2 = load i32, i32 addrspace(1)* %srcidx, align 4
  br label %exit
exit:
  ret void
}

; CHECK-LABEL: @test6
; CHECK-LABEL: entry
define void @test6(i32 addrspace(1)* noalias %src, i32 addrspace(1)* noalias %dst) local_unnamed_addr #0 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %srcidx = getelementptr inbounds i32, i32 addrspace(1)* %src, i64 %call
  %val = load i32, i32 addrspace(1)* %srcidx, align 4
  %dstidx = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %call
  ; CHECK: store i32 %val, i32 addrspace(1)* %dstidx, align 4, !nontemporal !3
  store i32 %val, i32 addrspace(1)* %dstidx, align 4
  %0 = icmp ne i32 %val, 32
  br i1 %0, label %b1, label %b2
b1:
  %val1 = load i32, i32 addrspace(1)* %srcidx, align 4
  br label %exit
b2:
  %val2 = load i32, i32 addrspace(1)* %srcidx, align 4
  br label %exit
exit:
  ret void
}

; CHECK-LABEL: @test7
; CHECK-LABEL: entry
define void @test7(i32 addrspace(1)* noalias %src, i32 addrspace(1)* noalias %dst) local_unnamed_addr #0 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %srcidx = getelementptr inbounds i32, i32 addrspace(1)* %src, i64 %call
  %val = load i32, i32 addrspace(1)* %srcidx, align 4
  %dstidx = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %call
  ; CHECK-NOT: !nontemporal
  store i32 %val, i32 addrspace(1)* %dstidx, align 4
  %0 = icmp ne i32 %val, 32
  br i1 %0, label %b1, label %b2
b1:
  %val1 = load i32, i32 addrspace(1)* %srcidx, align 4
  br label %exit
b2:
  %val2 = load i32, i32 addrspace(1)* %dstidx, align 4
  br label %exit
exit:
  ret void
}

define void @hello(float* noalias %src, float* noalias %dst) #0 {
entry:
  %0 = load float, float* %dst, align 4
  ret void
}

; CHECK-LABEL: @test8
; CHECK-LABEL: entry
define void @test8(float* noalias %src, float* noalias %dst) #0 {
entry:
  tail call void @hello(float* noalias %src, float* noalias %dst)
  %0 = load float, float* %src, align 4
  %dstidx = getelementptr inbounds float, float* %dst, i64 7
  ; CHECK-NOT: !nontemporal
  store float %0, float* %dstidx, align 4
  ret void
}

define void @hello1(float* %src) #0 {
entry:
  %0 = load float, float* %src, align 4
  ret void
}

; CHECK-LABEL: @test9
; CHECK-LABEL: entry
define void @test9(float* noalias %src, float* noalias %dst) #0 {
entry:
  tail call void @hello1(float* %src)
  %0 = load float, float* %src, align 4
  %dstidx = getelementptr inbounds float, float* %dst, i64 7
  ; CHECK: !nontemporal
  store float %0, float* %dstidx, align 4
  ret void
}

; CHECK-LABEL: @test10
; CHECK-LABEL: entry
define void @test10(i32 addrspace(1)* noalias %src, i32 addrspace(1)* noalias %dst) local_unnamed_addr #0 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %srcidx = getelementptr inbounds i32, i32 addrspace(1)* %src, i64 %call
  %dstidx = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %call
  %val = atomicrmw add i32 addrspace(1)* %srcidx, i32 2 monotonic
  ; CHECK: !nontemporal
  store i32 %val, i32 addrspace(1)* %dstidx, align 4
  ret void
}

; CHECK-LABEL: @test11
; CHECK-LABEL: entry
define void @test11(i32 addrspace(1)* noalias %src, i32 addrspace(1)* noalias %dst) local_unnamed_addr #0 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #3
  %srcidx = getelementptr inbounds i32, i32 addrspace(1)* %src, i64 %call
  %dstidx = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %call
  %val = atomicrmw add i32 addrspace(1)* %dstidx, i32 2 monotonic
  ; CHECK-NOT: !nontemporal
  store i32 %val, i32 addrspace(1)* %dst, align 4
  ret void
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-sign
ed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"=
"false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-pro
tector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent nounwind readnone }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}

; DEBUGIFY-NOT: WARNING
