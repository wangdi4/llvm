;;; Test that builtins with pointers in generic address space can be
;;; imported correctly.

; RUN: SATest -BUILD --config=%s.cfg -tsize=0 --dump-llvm-file - | FileCheck %s
;
; CHECK: call float @_Z5fractfPU3AS4f
; CHECK: call float @_Z5frexpfPU3AS4i
; CHECK: call float @_Z8lgamma_rfPU3AS4i
; CHECK: call float @_Z4modffPU3AS4f
; CHECK: call float @_Z6remquoffPU3AS4i
; CHECK: call float @_Z6sincosfPU3AS4f
; CHECK: call <2 x float> @_Z6vload2mPU3AS4Kf
; CHECK: call float @_Z10vload_halfmPU3AS4KDh
; CHECK: call void @_Z7vstore2Dv2_fmPU3AS4f
; CHECK: call void @_Z11vstore_halffmPU3AS4Dh
; CHECK: call void @_Z15vstore_half_rtefmPU3AS4Dh
; CHECK: call void @_Z15vstore_half_rtzfmPU3AS4Dh
; CHECK: call void @_Z15vstore_half_rtpfmPU3AS4Dh
; CHECK: call void @_Z15vstore_half_rtnfmPU3AS4Dh

; CHECK: Test program was successfully built.
