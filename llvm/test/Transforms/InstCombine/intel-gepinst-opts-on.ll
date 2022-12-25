; RUN: opt -passes=instcombine -aa-pipeline=basic-aa -instcombine-preserve-for-dtrans=false < %s -S 2>&1 | FileCheck %s

; Check that only one original GEPs is retained and one byte-flattened GEP
; is generated when -instcombine-preserve-for-dtrans=false.
; NOTE: The first call was changed to an unknown extern function bar because
; the community no longer does the GEP merging in the first case when the
; value operand of the store is a memory operation.

%struct.lzma_next_coder = type { i8*, i32, i32 }
%struct.lzma_coder = type { i32, i32, %struct.lzma_next_coder* }

declare dso_local noalias i8* @bar(i64)
declare dso_local noalias i8* @malloc(i64) #0


define dso_local void @foo(%struct.lzma_next_coder* %next) local_unnamed_addr {
  %call = call noalias i8* @bar(i64 16)
  %coder = getelementptr inbounds %struct.lzma_next_coder, %struct.lzma_next_coder* %next, i32 0, i32 0
  store i8* %call, i8** %coder, align 8
  %t0 = bitcast i8* %call to %struct.lzma_coder*
  %myint1 = getelementptr inbounds %struct.lzma_coder, %struct.lzma_coder* %t0, i32 0, i32 0
  store i32 15, i32* %myint1, align 8
  %call2 = call noalias i8* @malloc(i64 16)
  %t1 = bitcast i8* %call2 to %struct.lzma_next_coder*
  %t2 = load i8*, i8** %coder, align 8
  %t3 = bitcast i8* %t2 to %struct.lzma_coder*
  %next4 = getelementptr inbounds %struct.lzma_coder, %struct.lzma_coder* %t3, i32 0, i32 2
  store %struct.lzma_next_coder* %t1, %struct.lzma_next_coder** %next4, align 8
  ret void
}

attributes #0 = { inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

; CHECK:{{.*}}getelementptr inbounds %struct.lzma_next_coder{{.*}}
; CHECK:{{.*}}getelementptr inbounds i8, i8*{{.*}}
