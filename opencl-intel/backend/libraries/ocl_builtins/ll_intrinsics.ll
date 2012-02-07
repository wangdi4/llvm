; ModuleID = '<stdin>'

define <1 x i16> @zext_v1i8_v1i16(<1 x i8> %x) {
  %1 = zext <1 x i8> %x to <1 x i16>
  ret <1 x i16> %1
}

define <1 x i16> @sext_v1i8_v1i16(<1 x i8> %x) {
  %1 = sext <1 x i8> %x to <1 x i16>
  ret <1 x i16> %1
}

define <1 x i8> @trunc_v1i16_v1i8(<1 x i16> %x) {
  %1 = trunc <1 x i16> %x to <1 x i8>
  ret <1 x i8> %1
}

define <1 x i32> @zext_v1i8_v1i32(<1 x i8> %x) {
  %1 = zext <1 x i8> %x to <1 x i32>
  ret <1 x i32> %1
}

define <1 x i32> @sext_v1i8_v1i32(<1 x i8> %x) {
  %1 = sext <1 x i8> %x to <1 x i32>
  ret <1 x i32> %1
}

define <1 x i8> @trunc_v1i32_v1i8(<1 x i32> %x) {
  %1 = trunc <1 x i32> %x to <1 x i8>
  ret <1 x i8> %1
}

define <1 x i64> @zext_v1i8_v1i64(<1 x i8> %x) {
  %1 = zext <1 x i8> %x to <1 x i64>
  ret <1 x i64> %1
}

define <1 x i64> @sext_v1i8_v1i64(<1 x i8> %x) {
  %1 = sext <1 x i8> %x to <1 x i64>
  ret <1 x i64> %1
}

define <1 x i8> @trunc_v1i64_v1i8(<1 x i64> %x) {
  %1 = trunc <1 x i64> %x to <1 x i8>
  ret <1 x i8> %1
}

define <1 x i32> @zext_v1i16_v1i32(<1 x i16> %x) {
  %1 = zext <1 x i16> %x to <1 x i32>
  ret <1 x i32> %1
}

define <1 x i32> @sext_v1i16_v1i32(<1 x i16> %x) {
  %1 = sext <1 x i16> %x to <1 x i32>
  ret <1 x i32> %1
}

define <1 x i16> @trunc_v1i32_v1i16(<1 x i32> %x) {
  %1 = trunc <1 x i32> %x to <1 x i16>
  ret <1 x i16> %1
}

define <1 x i64> @zext_v1i16_v1i64(<1 x i16> %x) {
  %1 = zext <1 x i16> %x to <1 x i64>
  ret <1 x i64> %1
}

define <1 x i64> @sext_v1i16_v1i64(<1 x i16> %x) {
  %1 = sext <1 x i16> %x to <1 x i64>
  ret <1 x i64> %1
}

define <1 x i16> @trunc_v1i64_v1i16(<1 x i64> %x) {
  %1 = trunc <1 x i64> %x to <1 x i16>
  ret <1 x i16> %1
}

define <1 x i64> @zext_v1i32_v1i64(<1 x i32> %x) {
  %1 = zext <1 x i32> %x to <1 x i64>
  ret <1 x i64> %1
}

define <1 x i64> @sext_v1i32_v1i64(<1 x i32> %x) {
  %1 = sext <1 x i32> %x to <1 x i64>
  ret <1 x i64> %1
}

define <1 x i32> @trunc_v1i64_v1i32(<1 x i64> %x) {
  %1 = trunc <1 x i64> %x to <1 x i32>
  ret <1 x i32> %1
}

define <2 x i16> @zext_v2i8_v2i16(<2 x i8> %x) {
  %1 = zext <2 x i8> %x to <2 x i16>
  ret <2 x i16> %1
}

define <2 x i16> @sext_v2i8_v2i16(<2 x i8> %x) {
  %1 = sext <2 x i8> %x to <2 x i16>
  ret <2 x i16> %1
}

define <2 x i8> @trunc_v2i16_v2i8(<2 x i16> %x) {
  %1 = trunc <2 x i16> %x to <2 x i8>
  ret <2 x i8> %1
}

define <2 x i32> @zext_v2i8_v2i32(<2 x i8> %x) {
  %1 = zext <2 x i8> %x to <2 x i32>
  ret <2 x i32> %1
}

define <2 x i32> @sext_v2i8_v2i32(<2 x i8> %x) {
  %1 = sext <2 x i8> %x to <2 x i32>
  ret <2 x i32> %1
}

define <2 x i8> @trunc_v2i32_v2i8(<2 x i32> %x) {
  %1 = trunc <2 x i32> %x to <2 x i8>
  ret <2 x i8> %1
}

define <2 x i64> @zext_v2i8_v2i64(<2 x i8> %x) {
  %1 = zext <2 x i8> %x to <2 x i64>
  ret <2 x i64> %1
}

define <2 x i64> @sext_v2i8_v2i64(<2 x i8> %x) {
  %1 = sext <2 x i8> %x to <2 x i64>
  ret <2 x i64> %1
}

define <2 x i8> @trunc_v2i64_v2i8(<2 x i64> %x) {
  %1 = trunc <2 x i64> %x to <2 x i8>
  ret <2 x i8> %1
}

define <2 x i32> @zext_v2i16_v2i32(<2 x i16> %x) {
  %1 = zext <2 x i16> %x to <2 x i32>
  ret <2 x i32> %1
}

define <2 x i32> @sext_v2i16_v2i32(<2 x i16> %x) {
  %1 = sext <2 x i16> %x to <2 x i32>
  ret <2 x i32> %1
}

define <2 x i16> @trunc_v2i32_v2i16(<2 x i32> %x) {
  %1 = trunc <2 x i32> %x to <2 x i16>
  ret <2 x i16> %1
}

define <2 x i64> @zext_v2i16_v2i64(<2 x i16> %x) {
  %1 = zext <2 x i16> %x to <2 x i64>
  ret <2 x i64> %1
}

define <2 x i64> @sext_v2i16_v2i64(<2 x i16> %x) {
  %1 = sext <2 x i16> %x to <2 x i64>
  ret <2 x i64> %1
}

define <2 x i16> @trunc_v2i64_v2i16(<2 x i64> %x) {
  %1 = trunc <2 x i64> %x to <2 x i16>
  ret <2 x i16> %1
}

define <2 x i64> @zext_v2i32_v2i64(<2 x i32> %x) {
  %1 = zext <2 x i32> %x to <2 x i64>
  ret <2 x i64> %1
}

define <2 x i64> @sext_v2i32_v2i64(<2 x i32> %x) {
  %1 = sext <2 x i32> %x to <2 x i64>
  ret <2 x i64> %1
}

define <2 x i32> @trunc_v2i64_v2i32(<2 x i64> %x) {
  %1 = trunc <2 x i64> %x to <2 x i32>
  ret <2 x i32> %1
}

define <3 x i16> @zext_v3i8_v3i16(<3 x i8> %x) {
  %1 = zext <3 x i8> %x to <3 x i16>
  ret <3 x i16> %1
}

define <3 x i16> @sext_v3i8_v3i16(<3 x i8> %x) {
  %1 = sext <3 x i8> %x to <3 x i16>
  ret <3 x i16> %1
}

define <3 x i8> @trunc_v3i16_v3i8(<3 x i16> %x) {
  %1 = trunc <3 x i16> %x to <3 x i8>
  ret <3 x i8> %1
}

define <3 x i32> @zext_v3i8_v3i32(<3 x i8> %x) {
  %1 = zext <3 x i8> %x to <3 x i32>
  ret <3 x i32> %1
}

define <3 x i32> @sext_v3i8_v3i32(<3 x i8> %x) {
  %1 = sext <3 x i8> %x to <3 x i32>
  ret <3 x i32> %1
}

define <3 x i8> @trunc_v3i32_v3i8(<3 x i32> %x) {
  %1 = trunc <3 x i32> %x to <3 x i8>
  ret <3 x i8> %1
}

define <3 x i64> @zext_v3i8_v3i64(<3 x i8> %x) {
  %1 = zext <3 x i8> %x to <3 x i64>
  ret <3 x i64> %1
}

define <3 x i64> @sext_v3i8_v3i64(<3 x i8> %x) {
  %1 = sext <3 x i8> %x to <3 x i64>
  ret <3 x i64> %1
}

define <3 x i8> @trunc_v3i64_v3i8(<3 x i64> %x) {
  %1 = trunc <3 x i64> %x to <3 x i8>
  ret <3 x i8> %1
}

define <3 x i32> @zext_v3i16_v3i32(<3 x i16> %x) {
  %1 = zext <3 x i16> %x to <3 x i32>
  ret <3 x i32> %1
}

define <3 x i32> @sext_v3i16_v3i32(<3 x i16> %x) {
  %1 = sext <3 x i16> %x to <3 x i32>
  ret <3 x i32> %1
}

define <3 x i16> @trunc_v3i32_v3i16(<3 x i32> %x) {
  %1 = trunc <3 x i32> %x to <3 x i16>
  ret <3 x i16> %1
}

define <3 x i64> @zext_v3i16_v3i64(<3 x i16> %x) {
  %1 = zext <3 x i16> %x to <3 x i64>
  ret <3 x i64> %1
}

define <3 x i64> @sext_v3i16_v3i64(<3 x i16> %x) {
  %1 = sext <3 x i16> %x to <3 x i64>
  ret <3 x i64> %1
}

define <3 x i16> @trunc_v3i64_v3i16(<3 x i64> %x) {
  %1 = trunc <3 x i64> %x to <3 x i16>
  ret <3 x i16> %1
}

define <3 x i64> @zext_v3i32_v3i64(<3 x i32> %x) {
  %1 = zext <3 x i32> %x to <3 x i64>
  ret <3 x i64> %1
}

define <3 x i64> @sext_v3i32_v3i64(<3 x i32> %x) {
  %1 = sext <3 x i32> %x to <3 x i64>
  ret <3 x i64> %1
}

define <3 x i32> @trunc_v3i64_v3i32(<3 x i64> %x) {
  %1 = trunc <3 x i64> %x to <3 x i32>
  ret <3 x i32> %1
}

define <4 x i16> @zext_v4i8_v4i16(<4 x i8> %x) {
  %1 = zext <4 x i8> %x to <4 x i16>
  ret <4 x i16> %1
}

define <4 x i16> @sext_v4i8_v4i16(<4 x i8> %x) {
  %1 = sext <4 x i8> %x to <4 x i16>
  ret <4 x i16> %1
}

define <4 x i8> @trunc_v4i16_v4i8(<4 x i16> %x) {
  %1 = trunc <4 x i16> %x to <4 x i8>
  ret <4 x i8> %1
}

define <4 x i32> @zext_v4i8_v4i32(<4 x i8> %x) {
  %1 = zext <4 x i8> %x to <4 x i32>
  ret <4 x i32> %1
}

define <4 x i32> @sext_v4i8_v4i32(<4 x i8> %x) {
  %1 = sext <4 x i8> %x to <4 x i32>
  ret <4 x i32> %1
}

define <4 x i8> @trunc_v4i32_v4i8(<4 x i32> %x) {
  %1 = trunc <4 x i32> %x to <4 x i8>
  ret <4 x i8> %1
}

define <4 x i64> @zext_v4i8_v4i64(<4 x i8> %x) {
  %1 = zext <4 x i8> %x to <4 x i64>
  ret <4 x i64> %1
}

define <4 x i64> @sext_v4i8_v4i64(<4 x i8> %x) {
  %1 = sext <4 x i8> %x to <4 x i64>
  ret <4 x i64> %1
}

define <4 x i8> @trunc_v4i64_v4i8(<4 x i64> %x) {
  %1 = trunc <4 x i64> %x to <4 x i8>
  ret <4 x i8> %1
}

define <4 x i32> @zext_v4i16_v4i32(<4 x i16> %x) {
  %1 = zext <4 x i16> %x to <4 x i32>
  ret <4 x i32> %1
}

define <4 x i32> @sext_v4i16_v4i32(<4 x i16> %x) {
  %1 = sext <4 x i16> %x to <4 x i32>
  ret <4 x i32> %1
}

define <4 x i16> @trunc_v4i32_v4i16(<4 x i32> %x) {
  %1 = trunc <4 x i32> %x to <4 x i16>
  ret <4 x i16> %1
}

define <4 x i64> @zext_v4i16_v4i64(<4 x i16> %x) {
  %1 = zext <4 x i16> %x to <4 x i64>
  ret <4 x i64> %1
}

define <4 x i64> @sext_v4i16_v4i64(<4 x i16> %x) {
  %1 = sext <4 x i16> %x to <4 x i64>
  ret <4 x i64> %1
}

define <4 x i16> @trunc_v4i64_v4i16(<4 x i64> %x) {
  %1 = trunc <4 x i64> %x to <4 x i16>
  ret <4 x i16> %1
}

define <4 x i64> @zext_v4i32_v4i64(<4 x i32> %x) {
  %1 = zext <4 x i32> %x to <4 x i64>
  ret <4 x i64> %1
}

define <4 x i64> @sext_v4i32_v4i64(<4 x i32> %x) {
  %1 = sext <4 x i32> %x to <4 x i64>
  ret <4 x i64> %1
}

define <4 x i32> @trunc_v4i64_v4i32(<4 x i64> %x) {
  %1 = trunc <4 x i64> %x to <4 x i32>
  ret <4 x i32> %1
}

define <8 x i16> @zext_v8i8_v8i16(<8 x i8> %x) {
  %1 = zext <8 x i8> %x to <8 x i16>
  ret <8 x i16> %1
}

define <8 x i16> @sext_v8i8_v8i16(<8 x i8> %x) {
  %1 = sext <8 x i8> %x to <8 x i16>
  ret <8 x i16> %1
}

define <8 x i8> @trunc_v8i16_v8i8(<8 x i16> %x) {
  %1 = trunc <8 x i16> %x to <8 x i8>
  ret <8 x i8> %1
}

define <8 x i32> @zext_v8i8_v8i32(<8 x i8> %x) {
  %1 = zext <8 x i8> %x to <8 x i32>
  ret <8 x i32> %1
}

define <8 x i32> @sext_v8i8_v8i32(<8 x i8> %x) {
  %1 = sext <8 x i8> %x to <8 x i32>
  ret <8 x i32> %1
}

define <8 x i8> @trunc_v8i32_v8i8(<8 x i32> %x) {
  %1 = trunc <8 x i32> %x to <8 x i8>
  ret <8 x i8> %1
}

define <8 x i64> @zext_v8i8_v8i64(<8 x i8> %x) {
  %1 = zext <8 x i8> %x to <8 x i64>
  ret <8 x i64> %1
}

define <8 x i64> @sext_v8i8_v8i64(<8 x i8> %x) {
  %1 = sext <8 x i8> %x to <8 x i64>
  ret <8 x i64> %1
}

define <8 x i8> @trunc_v8i64_v8i8(<8 x i64> %x) {
  %1 = trunc <8 x i64> %x to <8 x i8>
  ret <8 x i8> %1
}

define <8 x i32> @zext_v8i16_v8i32(<8 x i16> %x) {
  %1 = zext <8 x i16> %x to <8 x i32>
  ret <8 x i32> %1
}

define <8 x i32> @sext_v8i16_v8i32(<8 x i16> %x) {
  %1 = sext <8 x i16> %x to <8 x i32>
  ret <8 x i32> %1
}

define <8 x i16> @trunc_v8i32_v8i16(<8 x i32> %x) {
  %1 = trunc <8 x i32> %x to <8 x i16>
  ret <8 x i16> %1
}

define <8 x i64> @zext_v8i16_v8i64(<8 x i16> %x) {
  %1 = zext <8 x i16> %x to <8 x i64>
  ret <8 x i64> %1
}

define <8 x i64> @sext_v8i16_v8i64(<8 x i16> %x) {
  %1 = sext <8 x i16> %x to <8 x i64>
  ret <8 x i64> %1
}

define <8 x i16> @trunc_v8i64_v8i16(<8 x i64> %x) {
  %1 = trunc <8 x i64> %x to <8 x i16>
  ret <8 x i16> %1
}

define <8 x i64> @zext_v8i32_v8i64(<8 x i32> %x) {
  %1 = zext <8 x i32> %x to <8 x i64>
  ret <8 x i64> %1
}

define <8 x i64> @sext_v8i32_v8i64(<8 x i32> %x) {
  %1 = sext <8 x i32> %x to <8 x i64>
  ret <8 x i64> %1
}

define <8 x i32> @trunc_v8i64_v8i32(<8 x i64> %x) {
  %1 = trunc <8 x i64> %x to <8 x i32>
  ret <8 x i32> %1
}

define <16 x i16> @zext_v16i8_v16i16(<16 x i8> %x) {
  %1 = zext <16 x i8> %x to <16 x i16>
  ret <16 x i16> %1
}

define <16 x i16> @sext_v16i8_v16i16(<16 x i8> %x) {
  %1 = sext <16 x i8> %x to <16 x i16>
  ret <16 x i16> %1
}

define <16 x i8> @trunc_v16i16_v16i8(<16 x i16> %x) {
  %1 = trunc <16 x i16> %x to <16 x i8>
  ret <16 x i8> %1
}

define <16 x i32> @zext_v16i8_v16i32(<16 x i8> %x) {
  %1 = zext <16 x i8> %x to <16 x i32>
  ret <16 x i32> %1
}

define <16 x i32> @sext_v16i8_v16i32(<16 x i8> %x) {
  %1 = sext <16 x i8> %x to <16 x i32>
  ret <16 x i32> %1
}

define <16 x i8> @trunc_v16i32_v16i8(<16 x i32> %x) {
  %1 = trunc <16 x i32> %x to <16 x i8>
  ret <16 x i8> %1
}

define <16 x i64> @zext_v16i8_v16i64(<16 x i8> %x) {
  %1 = zext <16 x i8> %x to <16 x i64>
  ret <16 x i64> %1
}

define <16 x i64> @sext_v16i8_v16i64(<16 x i8> %x) {
  %1 = sext <16 x i8> %x to <16 x i64>
  ret <16 x i64> %1
}

define <16 x i8> @trunc_v16i64_v16i8(<16 x i64> %x) {
  %1 = trunc <16 x i64> %x to <16 x i8>
  ret <16 x i8> %1
}

define <16 x i32> @zext_v16i16_v16i32(<16 x i16> %x) {
  %1 = zext <16 x i16> %x to <16 x i32>
  ret <16 x i32> %1
}

define <16 x i32> @sext_v16i16_v16i32(<16 x i16> %x) {
  %1 = sext <16 x i16> %x to <16 x i32>
  ret <16 x i32> %1
}

define <16 x i16> @trunc_v16i32_v16i16(<16 x i32> %x) {
  %1 = trunc <16 x i32> %x to <16 x i16>
  ret <16 x i16> %1
}

define <16 x i64> @zext_v16i16_v16i64(<16 x i16> %x) {
  %1 = zext <16 x i16> %x to <16 x i64>
  ret <16 x i64> %1
}

define <16 x i64> @sext_v16i16_v16i64(<16 x i16> %x) {
  %1 = sext <16 x i16> %x to <16 x i64>
  ret <16 x i64> %1
}

define <16 x i16> @trunc_v16i64_v16i16(<16 x i64> %x) {
  %1 = trunc <16 x i64> %x to <16 x i16>
  ret <16 x i16> %1
}

define <16 x i64> @zext_v16i32_v16i64(<16 x i32> %x) {
  %1 = zext <16 x i32> %x to <16 x i64>
  ret <16 x i64> %1
}

define <16 x i64> @sext_v16i32_v16i64(<16 x i32> %x) {
  %1 = sext <16 x i32> %x to <16 x i64>
  ret <16 x i64> %1
}

define <16 x i32> @trunc_v16i64_v16i32(<16 x i64> %x) {
  %1 = trunc <16 x i64> %x to <16 x i32>
  ret <16 x i32> %1
}
