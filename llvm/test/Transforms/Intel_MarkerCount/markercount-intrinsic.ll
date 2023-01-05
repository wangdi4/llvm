; REQUIRES: intel_feature_markercount
; RUN: opt %s -S | FileCheck %s

declare void @llvm.mark.loop.header()
declare void @llvm.mark.prolog()
declare void @llvm.mark.epilog()

; CHECK: declare void @llvm.mark.loop.header() #0
; CHECK: declare void @llvm.mark.prolog() #0
; CHECK: declare void @llvm.mark.epilog() #0
; CHECK: attributes #0 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }
