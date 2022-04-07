; AMX intrinsics support

define <256 x i32> @_tileloadd64_internal(i16 %row, i16 %col, i8 addrspace(4)* %buf, i64 %stride) {
  %p = addrspacecast i8 addrspace(4)* %buf to i8*
  %t = call x86_amx @llvm.x86.tileloadd64.internal(i16 %row, i16 %col, i8* %p, i64 %stride)
  %res = bitcast x86_amx %t to <256 x i32>
  ret <256 x i32> %res
}

define <256 x i32> @_tdpbssd_internal(i16 %m, i16 %n, i16 %k, <256 x i32> %c, <256 x i32> %a, <256 x i32> %b) {
  %c1 = bitcast <256 x i32> %c to x86_amx
  %a1 = bitcast <256 x i32> %a to x86_amx
  %b1 = bitcast <256 x i32> %b to x86_amx
  %t = call x86_amx @llvm.x86.tdpbssd.internal(i16 %m, i16 %n, i16 %k, x86_amx %c1, x86_amx %a1, x86_amx %b1)
  %res = bitcast x86_amx %t to <256 x i32>
  ret <256 x i32> %res
}

define <256 x i32> @_tdpbsud_internal(i16 %m, i16 %n, i16 %k, <256 x i32> %c, <256 x i32> %a, <256 x i32> %b) {
  %c1 = bitcast <256 x i32> %c to x86_amx
  %a1 = bitcast <256 x i32> %a to x86_amx
  %b1 = bitcast <256 x i32> %b to x86_amx
  %t = call x86_amx @llvm.x86.tdpbsud.internal(i16 %m, i16 %n, i16 %k, x86_amx %c1, x86_amx %a1, x86_amx %b1)
  %res = bitcast x86_amx %t to <256 x i32>
  ret <256 x i32> %res
}

define <256 x i32> @_tdpbusd_internal(i16 %m, i16 %n, i16 %k, <256 x i32> %c, <256 x i32> %a, <256 x i32> %b) {
  %c1 = bitcast <256 x i32> %c to x86_amx
  %a1 = bitcast <256 x i32> %a to x86_amx
  %b1 = bitcast <256 x i32> %b to x86_amx
  %t = call x86_amx @llvm.x86.tdpbusd.internal(i16 %m, i16 %n, i16 %k, x86_amx %c1, x86_amx %a1, x86_amx %b1)
  %res = bitcast x86_amx %t to <256 x i32>
  ret <256 x i32> %res
}

define <256 x i32> @_tdpbuud_internal(i16 %m, i16 %n, i16 %k, <256 x i32> %c, <256 x i32> %a, <256 x i32> %b) {
  %c1 = bitcast <256 x i32> %c to x86_amx
  %a1 = bitcast <256 x i32> %a to x86_amx
  %b1 = bitcast <256 x i32> %b to x86_amx
  %t = call x86_amx @llvm.x86.tdpbuud.internal(i16 %m, i16 %n, i16 %k, x86_amx %c1, x86_amx %a1, x86_amx %b1)
  %res = bitcast x86_amx %t to <256 x i32>
  ret <256 x i32> %res
}

define <256 x i32> @_tdpbf16ps_internal(i16 %m, i16 %n, i16 %k, <256 x i32> %c, <256 x i32> %a, <256 x i32> %b) {
  %c1 = bitcast <256 x i32> %c to x86_amx
  %a1 = bitcast <256 x i32> %a to x86_amx
  %b1 = bitcast <256 x i32> %b to x86_amx
  %t = call x86_amx @llvm.x86.tdpbf16ps.internal(i16 %m, i16 %n, i16 %k, x86_amx %c1, x86_amx %a1, x86_amx %b1)
  %res = bitcast x86_amx %t to <256 x i32>
  ret <256 x i32> %res
}

define void @_tilestored64_internal(i16 %row, i16 %col, i8 addrspace(4)* %buf, i64 %stride, <256 x i32> %a) {
  %p = addrspacecast i8 addrspace(4)* %buf to i8*
  %b = bitcast <256 x i32> %a to x86_amx
  call void @llvm.x86.tilestored64.internal(i16 %row, i16 %col, i8* %p, i64 %stride, x86_amx %b)
  ret void
}

declare x86_amx @llvm.x86.tileloadd64.internal(i16, i16, i8*, i64)
declare x86_amx @llvm.x86.tdpbssd.internal(i16, i16, i16, x86_amx, x86_amx, x86_amx)
declare x86_amx @llvm.x86.tdpbsud.internal(i16, i16, i16, x86_amx, x86_amx, x86_amx)
declare x86_amx @llvm.x86.tdpbusd.internal(i16, i16, i16, x86_amx, x86_amx, x86_amx)
declare x86_amx @llvm.x86.tdpbuud.internal(i16, i16, i16, x86_amx, x86_amx, x86_amx)
declare x86_amx @llvm.x86.tdpbf16ps.internal(i16, i16, i16, x86_amx, x86_amx, x86_amx)
declare void @llvm.x86.tilestored64.internal(i16, i16, i8*, i64, x86_amx)
