; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare {<16 x float>, <16 x i1>} @llvm.x86.mic.gatherd.step.ps(<16 x float>, <16 x i1>, <16 x i32>, float*, i32);
declare {<16 x i32>, <16 x i1>} @llvm.x86.mic.gatherd.step.pi(<16 x i32>, <16 x i1>, <16 x i32>, i32*, i32);
declare i32 @llvm.x86.mic.mask16.to.int(<16 x i1>)

define <16 x float> @gatherf(<16 x float> %oldvals, <16 x i1> %mask, <16 x i32> %indices, float* %base) nounwind alwaysinline {
; KNF: @gatherf
; KNF: vgatherd	(%r{{[a-z0-9]+}},%v{{[0-9]+}},4), %v{{[0-9]+}}{[[R1:%k[0-9]+]]}
; KNF: vkortest	[[R1]], [[R1]]

start:
        br label %loop

loop:
        %newvals = phi <16 x float> [ %oldvals, %start ], [ %stepvals, %loop ]
        %newmask = phi <16 x i1> [ %mask, %start ], [ %stepmask, %loop ]
        %g = call {<16 x float>, <16 x i1>} @llvm.x86.mic.gatherd.step.ps(<16 x float> %newvals, <16 x i1> %newmask, <16 x i32> %indices, float* %base, i32 4) ; last arg is indices scale, which may be 1, 2, 4, or 8
        %stepvals = extractvalue {<16 x float>, <16 x i1>}  %g, 0
        %stepmask = extractvalue {<16 x float>, <16 x i1>}  %g, 1
        %maski = call i32 @llvm.x86.mic.mask16.to.int(<16 x i1> %stepmask)
        %cond = icmp eq i32 %maski, 0
        br i1 %cond, label %end, label %loop

end:
        ret <16 x float> %stepvals
}

define <16 x i32> @gatheri(<16 x i32> %oldvals, <16 x i1> %mask, <16 x i32> %indices, i32* %base) nounwind alwaysinline {
; KNF: @gatheri
; KNF: vgatherd	(%r{{[a-z0-9]+}},%v{{[0-9]+}},4), %v{{[0-9]+}}{[[R2:%k[0-9]+]]}
; KNF: vkortest	[[R2]], [[R2]]

start:
        br label %loop

loop:
        %newvals = phi <16 x i32> [ %oldvals, %start ], [ %stepvals, %loop ]
        %newmask = phi <16 x i1> [ %mask, %start ], [ %stepmask, %loop ]
        %g = call {<16 x i32>, <16 x i1>} @llvm.x86.mic.gatherd.step.pi(<16 x i32> %newvals, <16 x i1> %newmask, <16 x i32> %indices, i32* %base, i32 4) ; last arg is indices scale, which may be 1, 2, 4, or 8
        %stepvals = extractvalue {<16 x i32>, <16 x i1>}  %g, 0
        %stepmask = extractvalue {<16 x i32>, <16 x i1>}  %g, 1
        %maski = call i32 @llvm.x86.mic.mask16.to.int(<16 x i1> %stepmask)
        %cond = icmp eq i32 %maski, 0
        br i1 %cond, label %end, label %loop

end:
        ret <16 x i32> %stepvals
}
