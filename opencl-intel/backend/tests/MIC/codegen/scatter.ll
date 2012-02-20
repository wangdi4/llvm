; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare <16 x i1> @llvm.x86.mic.scatterd.step.ps(float*, <16 x i1>, <16 x i32>, <16 x float>, i32);
declare <16 x i1> @llvm.x86.mic.scatterd.step.pi(i32*, <16 x i1>, <16 x i32>, <16 x i32>, i32);
declare i32 @llvm.x86.mic.mask16.to.int(<16 x i1>)

define void @scatterf(<16 x float> %vals, <16 x i1> %mask, <16 x i32> %indices, float* %base) nounwind alwaysinline {
; KNF: @scatterf
; KNF: vscatterd	%v{{[0-9]+}}, (%r{{[a-z0-9]+}},%v{{[0-9]+}},4) {[[R1:%k[0-9]+]]}
; KNF: vkortest	[[R1]], [[R1]]

start:
        br label %loop

loop:
        %newmask = phi <16 x i1> [ %mask, %start ], [ %stepmask, %loop ]
        %stepmask = call <16 x i1> @llvm.x86.mic.scatterd.step.ps(float* %base, <16 x i1> %newmask, <16 x i32> %indices, <16 x float> %vals, i32 4) ; last arg is indices scale, which may be 1, 2, 4, or 8
        %maski = call i32 @llvm.x86.mic.mask16.to.int(<16 x i1> %stepmask)
        %cond = icmp eq i32 %maski, 0
        br i1 %cond, label %end, label %loop

end:
        ret void
}

define void @scatteri(<16 x i32> %vals, <16 x i1> %mask, <16 x i32> %indices, i32* %base) nounwind alwaysinline {
; KNF: @scatteri
; KNF: vscatterd	%v{{[0-9]+}}, (%r{{[a-z0-9]+}},%v{{[0-9]+}},4) {[[R2:%k[0-9]+]]}
; KNF: vkortest	[[R2]], [[R2]]

start:
        br label %loop

loop:
        %newmask = phi <16 x i1> [ %mask, %start ], [ %stepmask, %loop ]
        %stepmask = call <16 x i1> @llvm.x86.mic.scatterd.step.pi(i32* %base, <16 x i1> %newmask, <16 x i32> %indices, <16 x i32> %vals, i32 4) ; last arg is indices scale, which may be 1, 2, 4, or 8
        %maski = call i32 @llvm.x86.mic.mask16.to.int(<16 x i1> %stepmask)
        %cond = icmp eq i32 %maski, 0
        br i1 %cond, label %end, label %loop

end:
        ret void
}
