==============================================================================
Intel® OpenCL™ CPU Compiler vectorization factor attribute extension proposal
==============================================================================


Overview
--------


The goal of this extension is to allow programmers to specify vector length
the kernel should be vectorized to.  This information may be used by the
compiler to generate more optimal code.

The extension name is “cl_intel_vec_len_hint”.

For the device that supports the extension function clGetDeviceInfo with
parameter CL_DEVICE_EXTENSION should return space separated list of
extensions names that contains “cl_intel_vec_len_hint”.

New OpenCL C Optional Attribute Qualifiers
    Optional __kernel qualifier:

          __attribute__((intel_vec_len_hint(<uint>)))


Add to Section 6.7.2 - "Optional Attribute Qualifiers"

    "The optional __attribute__((intel_vec_len_hint(<uint>))) can be used
    to provide a hint to the compiler that the kernel performs the best if
    vectorized to the specified vector length.


    The lengths accepted by the attribute are:
    0 - the compiler makes heuristic–based decision whether to vectorize
    the kernel, and if so which vector length to use (default behavior).
    1 - No vectorization by the compiler. Explicit vector data types in
    kernels are left intact.
    4,8,16 - Disables heuristic and vectorizes to the length of 4,8,16
    respectively


    Other values are invalid. An error will be reported during compilation.
    Some values may not be supported on specific architecture. In such
    cases the compiler emits warning, ignores the value specified and
    chooses vectorization length based on heuristic decision.


    Note that if the __attribute__((intel_vec_len_hint()))and
    __attribute__((vec_type_hint())) attributes are specified
    simultaneously the compiler
    ignores vec_type_hint attribute.”


Examples
--------


This section lists a few examples illustrating uses of the attribute.

Vectorize to the length of 8
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c

      #if defined(cl_intel_vec_len_hint)
      #pragma OPENCL EXTENSION cl_intel_vec_len_hint: enable
      #endif


      #if defined(cl_intel_vec_len_hint)
      __attribute__((intel_vec_len_hint(8)))
      #endif
      __kernel void kernel1(…) {
          ...
      }

Disable vectorization
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c

      #if defined(cl_intel_vec_len_hint)
      #pragma OPENCL EXTENSION cl_intel_vec_len_hint: enable
      #endif


      #if defined(cl_intel_vec_len_hint)
      __attribute__((intel_vec_len_hint(1)))
      #endif
      __kernel void kernel2(...) {
          ...
      }


Specify intel_vec_len_hint and vec_type_hint simultaneously
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
In this situation compiler will ignore vec_type_hint and vectorize to the length of 4.

.. code-block:: c

      #if defined(cl_intel_vec_len_hint)
      #pragma OPENCL EXTENSION cl_intel_vec_len_hint: enable
      #endif


      #if defined(cl_intel_vec_len_hint)
      __attribute__((intel_vec_len_hint(4)))
      #endif
      __attribute__((vec_type_hint (float8)))
      __kernel void kernel3(...) {
          ...
      }
