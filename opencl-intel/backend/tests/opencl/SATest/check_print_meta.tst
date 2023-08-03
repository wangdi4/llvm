; RUN: SATest -VAL -config=%s.cfg -noref -verbose 2>&1 | FileCheck %s
; CHECK: Number of arguments: 4
; CHECK: Argument 0 (Buffer)
; CHECK:     Length          : 1
; CHECK:     Type            : struct
; CHECK:     Element size(B) : 64
; CHECK:     Number of subtypes : 2
; CHECK:     {
; CHECK:         Type            : struct
; CHECK:         Element size(B) : 32
; CHECK:         Offset          : 0
; CHECK:         Number of subtypes : 2
; CHECK:         {
; CHECK:             Type            : f64
; CHECK:             Element size(B) : 8
; CHECK:             Offset          : 0
; CHECK:             Type            : vector
; CHECK:             Element size(B) : 16
; CHECK:             Offset          : 16
; CHECK:             VectorType      : u32 x 4
; CHECK:         }
; CHECK:         Type            : struct
; CHECK:         Element size(B) : 32
; CHECK:         Offset          : 32
; CHECK:         Number of subtypes : 2
; CHECK:         {
; CHECK:             Type            : f64
; CHECK:             Element size(B) : 8
; CHECK:             Offset          : 0
; CHECK:             Type            : vector
; CHECK:             Element size(B) : 16
; CHECK:             Offset          : 16
; CHECK:             VectorType      : u32 x 4
; CHECK:         }
; CHECK:     }
; CHECK: Argument 1 (Image)
; CHECK:     Image type      : CL_MEM_OBJECT_IMAGE2D
; CHECK:     Image size(B)   : 10000
; CHECK:     Element size(B) : 4
; CHECK:     Channel type    : CL_UNORM_INT8
; CHECK:     Channel order   : CL_RGBA
; CHECK:     Width           : 50
; CHECK:     Height          : 50
; CHECK:     Row             : 200
; CHECK:     Slice           : 0
; CHECK: Argument 2 (Buffer)
; CHECK:     Length          : 1
; CHECK:     Type            : u64
; CHECK:     Element size(B) : 8
; CHECK: Argument 3 (Buffer)
; CHECK:     Length          : 1
; CHECK:     Type            : vector
; CHECK:     Element size(B) : 32
; CHECK:     VectorType      : u32 x 8
