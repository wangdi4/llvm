; RUN: llvm-as < %s | llvm-dis | llvm-as > /dev/null
; RUN: verify-uselistorder %s

declare <4 x i32> @__svml_urem4_e9(<4 x i32> %0, <4 x i32> %1)

define <4 x i32> @urem4(<4 x i32> %in_0, <4 x i32> %in_1) {
  %res = call svml_unified_cc <4 x i32> @__svml_urem4_e9(<4 x i32> %in_0, <4 x i32> %in_1)
  ret <4 x i32> %res
}

declare <4 x i64> @__svml_u64rem4_l9(<4 x i64> %0, <4 x i64> %1)

define <4 x i64> @u64rem4(<4 x i64> %in_0, <4 x i64> %in_1) {
  %res = call svml_unified_cc_256 <4 x i64> @__svml_u64rem4_l9(<4 x i64> %in_0, <4 x i64> %in_1)
  ret <4 x i64> %res
}

declare <8 x i64> @__svml_u64rem8_z0(<8 x i64> %0, <8 x i64> %1)

define <8 x i64> @u64rem8(<8 x i64> %in_0, <8 x i64> %in_1) {
  %res = call svml_unified_cc_512 <8 x i64> @__svml_u64rem8_z0(<8 x i64> %in_0, <8 x i64> %in_1)
  ret <8 x i64> %res
}
