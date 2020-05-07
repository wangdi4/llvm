;;; Test that builtins with pointers in generic address space can be
;;; imported correctly.

; RUN: SATest -BUILD --config=%s.cfg -tsize=0 --dump-llvm-file - | FileCheck %s
;
; All these builtins are supposed to be inlined or replaced with SVML
; CHECK-NOT: @_Z10vload_halfmPU3AS4KDh
; CHECK-NOT: @_Z11vstore_halffmPU3AS4Dh
; CHECK-NOT: @_Z12vstorea_halffmPU3AS4Dh
; CHECK-NOT: @_Z15vstore_half_rtefmPU3AS4Dh
; CHECK-NOT: @_Z15vstore_half_rtnfmPU3AS4Dh
; CHECK-NOT: @_Z15vstore_half_rtpfmPU3AS4Dh
; CHECK-NOT: @_Z15vstore_half_rtzfmPU3AS4Dh
; CHECK-NOT: @_Z16vstorea_half_rtefmPU3AS4Dh
; CHECK-NOT: @_Z16vstorea_half_rtnfmPU3AS4Dh
; CHECK-NOT: @_Z16vstorea_half_rtpfmPU3AS4Dh
; CHECK-NOT: @_Z16vstorea_half_rtzfmPU3AS4Dh
; CHECK-NOT: @_Z4modffPU3AS4f
; CHECK-NOT: @_Z4modffPf
; CHECK-NOT: @_Z5fractfPU3AS4f
; CHECK-NOT: @_Z5fractfPf
; CHECK-NOT: @_Z5frexpfPU3AS4i
; CHECK-NOT: @_Z6remquoffPU3AS4i
; CHECK-NOT: @_Z6sincosfPU3AS4f
; CHECK-NOT: @_Z6sincosfPf
; CHECK-NOT: @_Z6vload2mPU3AS4Kf
; CHECK-NOT: @_Z7vstore2Dv2_fmPU3AS4f
; CHECK-NOT: @_Z8lgamma_rfPU3AS4i
