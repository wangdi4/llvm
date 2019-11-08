; XFAIL: *
; RUN: %oclopt -coerce-types -S %s -o - | FileCheck %s
; This test checks caller and callee patching that makes use of the coerced arguments

; ModuleID = 'main'
source_filename = "1"
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
; CHECK: [[BC:%[a-zA-Z0-9]+]] = bitcast %struct.OneWord* %OW to i64*
; CHECK-NEXT: [[LOAD:%[a-zA-Z0-9]+]] = load i64, i64* [[BC]]
; CHECK-NEXT: call i64 @oneWord(i64 [[LOAD]]
  %1 = bitcast %struct.TwoWords* %TW to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* %1) #3
  %call1 = call i64 @twoWords(%struct.TwoWords* byval(%struct.TwoWords) align 8 %TW) #4
; CHECK: [[BC:%[a-zA-Z0-9]+]] = bitcast %struct.TwoWords* %TW to %struct.TwoWords.coerce.0*
; CHECK-NEXT: [[GEP:%[a-zA-Z0-9]+]] = getelementptr %struct.TwoWords.coerce.0, %struct.TwoWords.coerce.0* [[BC]], i32 0, i32 0
; CHECK-NEXT: [[LOAD0:%[a-zA-Z0-9]+]] = load i64, i64* [[GEP]]
; CHECK-NEXT: [[GEP:%[a-zA-Z0-9]+]] = getelementptr %struct.TwoWords.coerce.0, %struct.TwoWords.coerce.0* [[BC]], i32 0, i32 1
; CHECK-NEXT: [[LOAD1:%[a-zA-Z0-9]+]] = load i64, i64* [[GEP]]
; CHECK-NEXT: call i64 @twoWords(i64 [[LOAD0]], i64 [[LOAD1]])
  %2 = bitcast %struct.TwoWords* %TW to i8*
  call void @llvm.lifetime.end.p0i8(i64 16, i8* %2) #3
  %3 = bitcast %struct.OneWord* %OW to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %3) #3
  ret void
}

; CHECK: define i64 @oneWord(i64 %arg.coerce.high)
; CHECK: [[ALLOCA:%[a-zA-Z0-9]+]] = alloca %struct.OneWord
; CHECK-NEXT: [[BC:%[a-zA-Z0-9]+]] = bitcast %struct.OneWord* [[ALLOCA]] to i64*
; CHECK-NEXT: store i64 %arg.coerce.high, i64* [[BC]]
; CHECK-NEXT: getelementptr inbounds %struct.OneWord, %struct.OneWord* [[ALLOCA]], i32 0, i32 0

; CHECK: define i64 @twoWords(i64 %arg.coerce.high, i64 %arg.coerce.low)
; CHECK: [[ALLOCA:%[a-zA-Z0-9]+]] = alloca %struct.TwoWords
; CHECK-NEXT: [[BC:%[a-zA-Z0-9]+]] = bitcast %struct.TwoWords* %0 to %struct.TwoWords.coerce*
; CHECK-NEXT: [[GEP:%[a-zA-Z0-9]+]] = getelementptr %struct.TwoWords.coerce, %struct.TwoWords.coerce* [[BC]], i32 0, i32 0
; CHECK-NEXT: store i64 %arg.coerce.high, i64* [[GEP]]
; CHECK-NEXT: [[GEP:%[a-zA-Z0-9]+]] = getelementptr %struct.TwoWords.coerce, %struct.TwoWords.coerce* [[BC]], i32 0, i32 1
; CHECK-NEXT: store i64 %arg.coerce.low, i64* [[GEP]]
; CHECK-NEXT: getelementptr inbounds %struct.TwoWords, %struct.TwoWords* [[ALLOCA]], i32 0, i32 0

; Function Attrs: argmemonly nounwind
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
!opencl.kernels = !{!4}

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
