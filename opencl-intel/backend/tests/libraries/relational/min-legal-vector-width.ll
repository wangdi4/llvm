; RUN: llvm-extract %libdir/../clbltfnshared.rtl -rfunc "__ocl_zext_v.{1,2}i32_v.{1,2}i64" -S -o - | FileCheck %s

; CHECK-DAG: define <16 x i64> @__ocl_zext_v16i32_v16i64(<16 x i32> %x){{.*}} [[ATTR_1024:#[0-9]+]] {
; CHECK-DAG: define i64 @__ocl_zext_v1i32_v1i64(i32 %x){{.*}} [[ATTR_NONE:#[0-9]+]] {
; CHECK-DAG: define <2 x i64> @__ocl_zext_v2i32_v2i64(<2 x i32> %x){{.*}} [[ATTR_128:#[0-9]+]] {
; CHECK-DAG: define <32 x i64> @__ocl_zext_v32i32_v32i64(<32 x i32> %x){{.*}} [[ATTR_2048:#[0-9]+]] {
; CHECK-DAG: define <3 x i64> @__ocl_zext_v3i32_v3i64(<3 x i32> %x){{.*}} [[ATTR_192:#[0-9]+]] {
; CHECK-DAG: define <4 x i64> @__ocl_zext_v4i32_v4i64(<4 x i32> %x){{.*}} [[ATTR_256:#[0-9]+]] {
; CHECK-DAG: define <64 x i64> @__ocl_zext_v64i32_v64i64(<64 x i32> %x){{.*}} [[ATTR_4096:#[0-9]+]] {
; CHECK-DAG: define <8 x i64> @__ocl_zext_v8i32_v8i64(<8 x i32> %x){{.*}} [[ATTR_512:#[0-9]+]] {

; CHECK-DAG: attributes [[ATTR_1024]] = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) "min-legal-vector-width"="1024" }
; CHECK-DAG: attributes [[ATTR_NONE]] = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) }
; CHECK-DAG: attributes [[ATTR_128]] = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) "min-legal-vector-width"="128" }
; CHECK-DAG: attributes [[ATTR_2048]] = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) "min-legal-vector-width"="2048" }
; CHECK-DAG: attributes [[ATTR_192]] = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) "min-legal-vector-width"="192" }
; CHECK-DAG: attributes [[ATTR_256]] = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) "min-legal-vector-width"="256" }
; CHECK-DAG: attributes [[ATTR_4096]] = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) "min-legal-vector-width"="4096" }
; CHECK-DAG: attributes [[ATTR_512]] = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) "min-legal-vector-width"="512" }
