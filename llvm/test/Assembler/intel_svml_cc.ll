; RUN: llvm-as < %s | llvm-dis | llvm-as > /dev/null
; RUN: verify-uselistorder %s

declare <4 x i64> @__svml_u64rem4_l9(<4 x i64> %0, <4 x i64> %1)

define <4 x i64> @u64rem(<4 x i64> %in_0, <4 x i64> %in_1) {
  %res = call svml_avx_cc <4 x i64> @__svml_u64rem4_l9(<4 x i64> %in_0, <4 x i64> %in_1)
  ret <4 x i64> %res
}

define <4 x i64> @u64rem_avx_impl(<4 x i64> %in_0, <4 x i64> %in_1) {
  %res = call svml_avx_avx_impl_cc <4 x i64> @__svml_u64rem4_l9(<4 x i64> %in_0, <4 x i64> %in_1)
  ret <4 x i64> %res
}
