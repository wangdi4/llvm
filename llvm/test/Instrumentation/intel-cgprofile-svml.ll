; RUN:  opt -S -cg-profile %s 2>&1 | FileCheck %s
; RUN:  opt -S -passes=cg-profile %s 2>&1 | FileCheck %s

; This test is to verify that references to __svml_* functions are not included
; in the "CG Profile" metadata. Calls to the __svml_* functions may be replaced
; with a call to a different function or scalarized during legalization by the
; MapIntrinToIml pass. Because the "CG Profile" metadata is just attached to the
; !llvm.module.flags metadata node, there is not a reasonable way to locate the
; information in the metadata for an affected call that MapIntrinToIml is
; changing. The only pass that can reasonably update the metadata is the
; GlobalDCE pass, but that is a Module level pass so is impractical to run
; during the Function pass pipeline that performs the MapIntrinToIml pass.

; CHECK: !"CG Profile"
; CHECK-NOT: !{{[0-9]+}} = !{{.*}} @__svml_pow16


%class.ggGamma = type { double, [16 x [256 x i8]], [256 x i8], [256 x i8] }

define void @"?SetGammaTables@ggGamma@@IEAAXXZ"() !prof !0 {
  %i0 = call fast svml_cc <16 x double> @__svml_pow16(<16 x double> zeroinitializer, <16 x double> zeroinitializer)
  ret void
}

define %class.ggGamma* @"??0ggGamma@@QEAA@N@Z"() !prof !0 {
  call void @"?SetGammaTables@ggGamma@@IEAAXXZ"()
  ret %class.ggGamma* null
}

declare <16 x double> @__svml_pow16(<16 x double>, <16 x double>)

!0 = !{!"function_entry_count", i64 3}
