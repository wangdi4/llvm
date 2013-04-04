; RUN: opt -mic-passes -inst-to-func-call -S %s | FileCheck %s
; RUN: opt -mic-passes -inst-to-func-call -S %s | grep "declare <16 x i64> @_Z14__ocl_reminderDv16_lS_(<16 x i64>, <16 x i64>)"
; RUN: opt -mic-passes -inst-to-func-call -S %s | grep "declare <16 x i64> @_Z14__ocl_reminderDv16_mS_(<16 x i64>, <16 x i64>)"
; RUN: opt -mic-passes -inst-to-func-call -S %s | grep "declare <16 x i32> @_Z14__ocl_reminderDv16_iS_(<16 x i32>, <16 x i32>)"
; RUN: opt -mic-passes -inst-to-func-call -S %s | grep "declare <16 x i32> @_Z14__ocl_reminderDv16_jS_(<16 x i32>, <16 x i32>)"

define <16 x i64> @sample_test_i64(<16 x i64> %x, <16 x i64> %y) nounwind
{
  %res = srem <16 x i64> %x, %y 
  ret <16 x i64> %res

; CHECK: @sample_test_i64
; CHECK: call <16 x i64> @_Z14__ocl_reminderDv16_lS_(<16 x i64> %x, <16 x i64> %y)
}

define <16 x i64> @sample_test_u64(<16 x i64> %x, <16 x i64> %y) nounwind
{
  %res = urem <16 x i64> %x, %y 
  ret <16 x i64> %res

; CHECK: @sample_test_u64
; CHECK: call <16 x i64> @_Z14__ocl_reminderDv16_mS_(<16 x i64> %x, <16 x i64> %y)
}

define <16 x i32> @sample_test_i32(<16 x i32> %x, <16 x i32> %y) nounwind
{
  %res = srem <16 x i32> %x, %y 
  ret <16 x i32> %res

; CHECK: @sample_test_i32
; CHECK: call <16 x i32> @_Z14__ocl_reminderDv16_iS_(<16 x i32> %x, <16 x i32> %y)
}

define <16 x i32> @sample_test_u32(<16 x i32> %x, <16 x i32> %y) nounwind
{
  %res = urem <16 x i32> %x, %y 
  ret <16 x i32> %res

; CHECK: @sample_test_u32
; CHECK: call <16 x i32> @_Z14__ocl_reminderDv16_jS_(<16 x i32> %x, <16 x i32> %y)
}
