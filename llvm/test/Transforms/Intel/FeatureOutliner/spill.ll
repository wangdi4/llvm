; RUN: opt -featureoutliner -S -consistent-vector-abi < %s | FileCheck %s
target triple = "x86_64-generic-generic"

declare <8 x float> @getvector(i32) #0
declare void @passvector(<8 x float>) #0
declare i1 @ret(<8 x float>) #0

; CHECK-LABEL: define void @spill(i1 %p, i1 %q) #1
define void @spill(i1 %p, i1 %q) #1 {
entry:
; CHECK: [[A1:%spill.*]] = alloca <8 x float>
; CHECK: [[A2:%spill.*]] = alloca <8 x float>
  br i1 %p, label %if, label %next
  
if:
; CHECK: call x86_fastcallcc void @spill_if
  %v0 = call <8 x float> @getvector(i32 0)
  %v1 = call <8 x float> @getvector(i32 1)
  br label %next
  
next:
  %phi0 = phi <8 x float> [ zeroinitializer, %entry ], [ %v0, %if ]
  %phi1 = phi <8 x float> [ zeroinitializer, %entry ], [ %v1, %if ]
  br i1 %p, label %if2, label %end
  
if2:
; CHECK: store {{.*}} <8 x float>* [[A1]]
; CHECK: store {{.*}} <8 x float>* [[A2]]
; CHECK: call x86_fastcallcc void @spill_if2.split(<8 x float>* [[A1]], <8 x float>* [[A2]])
  call void @passvector(<8 x float> %phi0)
  call void @passvector(<8 x float> %phi1)
  br label %end
  
end:
  ret void
}

; CHECK-LABEL: define void @nonphi(i1 %p, <8 x float>* %ptr) #1 {
define void @nonphi(i1 %p, <8 x float>* %ptr) #1 {
entry:
; CHECK: [[A3:%spill.*]] = alloca <8 x float>
  %v0 = load <8 x float>, <8 x float>* %ptr, align 4
  br i1 %p, label %if, label %end
  
if:
; CHECK: store <8 x float> %v0, <8 x float>* [[A3]]
; CHECK: call x86_fastcallcc void @nonphi_if.split(<8 x float>* [[A3]])
  call void @passvector(<8 x float> %v0)
  br label %end
    
end:
  ret void
}

; CHECK-LABEL: define void @internal(i1 %p) #1
define void @internal(i1 %p) #1 {
entry:
  br i1 %p, label %pre, label %end
  
pre:
  br label %header
  
header:
; CHECK: %targetBlock = call x86_fastcallcc i1 @internal_header.split(<8 x float>* %spillVec, <8 x float>* %v0.loc)
; CHECK: br i1 %targetBlock, label %header, label %end
  %phi0 = phi <8 x float> [ zeroinitializer, %pre ], [ %v0, %body ]
  %r = call i1 @ret(<8 x float> %phi0)
  br label %body
  
body:
  %v0 = call <8 x float> @getvector(i32 0)
  br i1 %r, label %header, label %end
  
end:
  ret void
}

; CHECK-LABEL: define internal x86_fastcallcc void @spill_if2.split(<8 x float>* %spillVec1, <8 x float>* %spillVec) #2 {
; CHECK: %reloadVec2 = load <8 x float>, <8 x float>* %spillVec1
; CHECK: %reloadVec = load <8 x float>, <8 x float>* %spillVec
; CHECK-DAG: call void @passvector(<8 x float> %reloadVec)
; CHECK-DAG: call void @passvector(<8 x float> %reloadVec2)

; CHECK-LABEL: define internal x86_fastcallcc void @nonphi_if.split(<8 x float>* %spillVec) #2 {
; CHECK: %reloadVec = load <8 x float>, <8 x float>* %spillVec
; CHECK: call void @passvector(<8 x float> %reloadVec)

; CHECK-LABEL: define internal x86_fastcallcc i1 @internal_header.split(<8 x float>* %spillVec, <8 x float>* %v0.out) #2 {
; CHECK: %r = call i1 @ret(<8 x float> %reloadVec)
; CHECK: br i1 %r, label %header.exitStub, label %end.exitStub

; CHECK: attributes #0 = { nounwind readnone }
; CHECK: attributes #1 = { "target-cpu"="x86-64" }
; CHECK: attributes #2 = { "target-cpu"="x86-64" "target-features"="+avx" }

attributes #0 = { nounwind readnone }
attributes #1 = { "target-cpu"="x86-64" }
