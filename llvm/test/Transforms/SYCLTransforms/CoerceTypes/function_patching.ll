; RUN: opt -passes=dpcpp-kernel-coerce-types -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-coerce-types -S %s -o - | FileCheck %s --check-prefixes=CHECK,NONOPAQUE

; RUN: opt -passes=dpcpp-kernel-coerce-types -opaque-pointers -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-coerce-types -opaque-pointers -S %s -o - | FileCheck %s --check-prefixes=CHECK,OPAQUE

; This test checks caller and callee patching that makes use of the coerced arguments

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct.OneWord = type { i64 }
%struct.TwoWords = type { [2 x i64] }
; CHECK: %struct.TwoWords.coerce.0 = type { i64, i64 }
; CHECK: %struct.TwoWords.coerce = type { i64, i64 }

; Function Attrs: convergent nounwind
define i64 @oneWord(%struct.OneWord* byval(%struct.OneWord) align 8 %arg) #0 {
entry:
  %a = getelementptr inbounds %struct.OneWord, %struct.OneWord* %arg, i32 0, i32 0
  %0 = load i64, i64* %a, align 8, !tbaa !5
  ret i64 %0
}

; Function Attrs: convergent nounwind
define i64 @twoWords(%struct.TwoWords* byval(%struct.TwoWords) align 8 %arg) #0 {
entry:
  %a = getelementptr inbounds %struct.TwoWords, %struct.TwoWords* %arg, i32 0, i32 0
  %arrayidx = getelementptr inbounds [2 x i64], [2 x i64]* %a, i64 0, i64 0
  %0 = load i64, i64* %arrayidx, align 8, !tbaa !10
  %a1 = getelementptr inbounds %struct.TwoWords, %struct.TwoWords* %arg, i32 0, i32 0
  %arrayidx2 = getelementptr inbounds [2 x i64], [2 x i64]* %a1, i64 0, i64 1
  %1 = load i64, i64* %arrayidx2, align 8, !tbaa !10
  %add = add nsw i64 %0, %1
  ret i64 %add
}

; Function Attrs: convergent nounwind
define void @test() #1 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 !kernel_arg_name !2 {
entry:
  %OW = alloca %struct.OneWord, align 8
  %TW = alloca %struct.TwoWords, align 8
  %0 = bitcast %struct.OneWord* %OW to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %0) #3
  %call = call i64 @oneWord(%struct.OneWord* byval(%struct.OneWord) align 8 %OW) #4
; NONOPAQUE: [[BC:%[a-zA-Z0-9]+]] = bitcast %struct.OneWord* %OW to i64*
; NONOPAQUE-NEXT: [[LOAD:%[a-zA-Z0-9]+]] = load i64, i64* [[BC]]
; NONOPAQUE-NEXT: call i64 @oneWord(i64 [[LOAD]]

; OPAQUE: call void @llvm.lifetime.start.p0(i64 8, ptr %{{.*}})
; OPAQUE-NEXT: [[LOAD:%[a-zA-Z0-9]+]] = load i64, ptr %OW
; OPAQUE-NEXT: call i64 @oneWord(i64 [[LOAD]]

  %1 = bitcast %struct.TwoWords* %TW to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* %1) #3
  %call1 = call i64 @twoWords(%struct.TwoWords* byval(%struct.TwoWords) align 8 %TW) #4
; NONOPAQUE: [[BC:%[a-zA-Z0-9]+]] = bitcast %struct.TwoWords* %TW to %struct.TwoWords.coerce.0*
; NONOPAQUE-NEXT: [[GEP0:%[a-zA-Z0-9]+]] = getelementptr %struct.TwoWords.coerce.0, %struct.TwoWords.coerce.0* [[BC]], i32 0, i32 0
; NONOPAQUE-NEXT: [[LOAD0:%[a-zA-Z0-9]+]] = load i64, i64* [[GEP0]]
; NONOPAQUE-NEXT: [[GEP1:%[a-zA-Z0-9]+]] = getelementptr %struct.TwoWords.coerce.0, %struct.TwoWords.coerce.0* [[BC]], i32 0, i32 1
; NONOPAQUE-NEXT: [[LOAD1:%[a-zA-Z0-9]+]] = load i64, i64* [[GEP1]]
; NONOPAQUE-NEXT: call i64 @twoWords(i64 [[LOAD0]], i64 [[LOAD1]])

; OPAQUE: call void @llvm.lifetime.start.p0(i64 16, ptr %{{.*}})
; OPAQUE-NEXT: [[GEP0:%[a-zA-Z0-9]+]] = getelementptr %struct.TwoWords.coerce.0, ptr %TW, i32 0, i32 0
; OPAQUE-NEXT: [[LOAD0:%[a-zA-Z0-9]+]] = load i64, ptr [[GEP0]]
; OPAQUE-NEXT: [[GEP1:%[a-zA-Z0-9]+]] = getelementptr %struct.TwoWords.coerce.0, ptr %TW, i32 0, i32 1
; OPAQUE-NEXT: [[LOAD1:%[a-zA-Z0-9]+]] = load i64, ptr [[GEP1]]
; OPAQUE-NEXT: call i64 @twoWords(i64 [[LOAD0]], i64 [[LOAD1]])

  %2 = bitcast %struct.TwoWords* %TW to i8*
  call void @llvm.lifetime.end.p0i8(i64 16, i8* %2) #3
  %3 = bitcast %struct.OneWord* %OW to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %3) #3
  ret void
}

; CHECK: define i64 @oneWord(i64 %arg.coerce.high)
; CHECK: [[ALLOCA:%[a-zA-Z0-9]+]] = alloca %struct.OneWord, align 8
; NONOPAQUE-NEXT: [[BC:%[a-zA-Z0-9]+]] = bitcast %struct.OneWord* [[ALLOCA]] to i64*
; NONOPAQUE-NEXT: store i64 %arg.coerce.high, i64* [[BC]]
; NONOPAQUE-NEXT: getelementptr inbounds %struct.OneWord, %struct.OneWord* [[ALLOCA]], i32 0, i32 0
; OPAQUE-NEXT: store i64 %arg.coerce.high, ptr [[ALLOCA]]
; OPAQUE-NEXT: getelementptr inbounds %struct.OneWord, ptr [[ALLOCA]], i32 0, i32 0

; CHECK: define i64 @twoWords(i64 %arg.coerce.high, i64 %arg.coerce.low)
; CHECK: [[ALLOCA:%[a-zA-Z0-9]+]] = alloca %struct.TwoWords, align 8
; NONOPAQUE-NEXT: [[BC:%[a-zA-Z0-9]+]] = bitcast %struct.TwoWords* %0 to %struct.TwoWords.coerce*
; NONOPAQUE-NEXT: [[GEP:%[a-zA-Z0-9]+]] = getelementptr %struct.TwoWords.coerce, %struct.TwoWords.coerce* [[BC]], i32 0, i32 0
; NONOPAQUE-NEXT: store i64 %arg.coerce.high, i64* [[GEP]]
; NONOPAQUE-NEXT: [[GEP:%[a-zA-Z0-9]+]] = getelementptr %struct.TwoWords.coerce, %struct.TwoWords.coerce* [[BC]], i32 0, i32 1
; NONOPAQUE-NEXT: store i64 %arg.coerce.low, i64* [[GEP]]
; NONOPAQUE-NEXT: getelementptr inbounds %struct.TwoWords, %struct.TwoWords* [[ALLOCA]], i32 0, i32 0
; OPAQUE-NEXT: [[GEP0:%[a-zA-Z0-9]+]] = getelementptr %struct.TwoWords.coerce, ptr [[ALLOCA]], i32 0, i32 0
; OPAQUE-NEXT: store i64 %arg.coerce.high, ptr [[GEP0]]
; OPAQUE-NEXT: [[GEP1:%[a-zA-Z0-9]+]] = getelementptr %struct.TwoWords.coerce, ptr [[ALLOCA]], i32 0, i32 1
; OPAQUE-NEXT: store i64 %arg.coerce.low, ptr [[GEP1]]
; OPAQUE-NEXT: getelementptr inbounds %struct.TwoWords, ptr [[ALLOCA]], i32 0, i32 0

define void @checkAllocaAddrspace(%struct.OneWord addrspace(4)* byval(%struct.OneWord) align 16 %arg) #0 {
; CHECK:  define void @checkAllocaAddrspace(i64 %arg.coerce.high)
; CHECK-NEXT:  [[ALLOCA:%[a-zA-Z0-9]+]] = alloca %struct.OneWord, align 16
; NONOPAQUE-NEXT: [[ADDRCAST:%[a-zA-Z0-9]+]] = addrspacecast %struct.OneWord* [[ALLOCA]] to %struct.OneWord addrspace(4)*
; NONOPAQUE-NEXT: [[BITCAST:%[a-zA-Z0-9]+]] = bitcast %struct.OneWord addrspace(4)* [[ADDRCAST]] to i64 addrspace(4)*
; NONOPAQUE-NEXT: store i64 %{{.*}}, i64 addrspace(4)* [[BITCAST]], align 8

; OPAQUE-NEXT: [[ADDRCAST:%[a-zA-Z0-9]+]] = addrspacecast ptr [[ALLOCA]] to ptr addrspace(4)
; OPAQUE-NEXT: store i64 %{{.*}}, ptr addrspace(4) [[ADDRCAST]], align 8
 ret void
}

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #2

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nounwind }
attributes #4 = { convergent }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!sycl.kernels = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"icx (ICX) dev.8.x.0"}
!4 = !{void ()* @test}
!5 = !{!6, !7, i64 0}
!6 = !{!"OneWord", !7, i64 0}
!7 = !{!"long", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!7, !7, i64 0}

; DEBUGIFY-NOT: WARNING
