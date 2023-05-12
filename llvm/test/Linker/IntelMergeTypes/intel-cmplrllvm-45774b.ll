; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans, asserts
; RUN: llvm-link -irmover-enable-merge-with-dtrans -irmover-trace-dtrans-metadata-loss -debug-only=irmover-dtrans-types -S %S/Inputs/intel-cmplrllvm-45774b.ll 2>&1 | FileCheck %s

; Check that types beginning with "%rtti." or "%eh." do not have DTrans
; metdata, but also do not inhibit DTrans Type Merging and are flagged
; as a special case (DoRTTIOrEHSpecial: T). We must tolerate this special
; case because the C/C++ front end will not emit DTrans metdata for them.

; CHECK-LABEL: Merging types from source module: {{.*}}intel-cmplrllvm-45774b.ll
; CHECK: Checking for metadata loss in source module:
; CHECK-LABEL: llvm::Type: %eh.CatchableTypeArray.3 = type { i32, [3 x ptr] }
; CHECK-NEXT: DTransType: None
; CHECK-LABEL: llvm::Type: %eh.ThrowInfo = type { i32, ptr, ptr, ptr }
; CHECK-NEXT: DTransType: None
; CHECK-LABEL: llvm::Type: %eh.CatchableType = type { i32, ptr, i32, i32, i32, i32, ptr }
; CHECK-NEXT: DTransType: None
; CHECK-LABEL: llvm::Type: %rtti.TypeDescriptor43 = type { ptr, ptr, [44 x i8] }
; CHECK-NEXT: DTransType: None
; CHECK-LABEL: Checking for metadata loss in destination module:
; CHECK-NEXT:  Skippable type: %eh.CatchableTypeArray.3 = type { i32, [3 x ptr] }
; CHECK-NEXT:  Skippable type: %eh.ThrowInfo = type { i32, ptr, ptr, ptr }
; CHECK-NEXT:  Skippable type: %eh.CatchableType = type { i32, ptr, i32, i32, i32, i32, ptr }
; CHECK-NEXT:  Skippable type: %rtti.TypeDescriptor43 = type { ptr, ptr, [44 x i8] }
; CHECK-LBAEL: IsMappingByDTransInfoEnabled: T
; CHECK-LABEL: DoRTTIOrEHSpecial: T
; end INTEL_FEATURE_SW_DTRANS
