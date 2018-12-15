This directory is for Intel-added header files.

If you add a new header file you may put it into this directory
and name it without "Intel_" prefix, because Intel_Headers directory
name is enough to classify files in this directory as Intel-added.

Files from this directory are copied during build/install so that
Intel_Headers is dropped from the destination path.

A few examples regarding adding new Intel header files:
  * Create Intel_Headers/newisaintrin.h file; add it into INTEL_HEADERS
    list in ../CMakeLists.txt as Intel_Headers/newisaintrin.h;
    add "#include <newisaintrin.h>" in ../immintrin.h.
    During build/install newisaintrin.h and immintrin.h will be copied
    to the same location, so that the include directive will work.
  * Create Intel_Headers/extended_support/vector_support.h;
    add it into INTEL_HEADERS list in ../CMakeLists.txt as
    Intel_Headers/extended_support/vector_support.h;
    use "#include <extended_support/vector_support.h>" in programs.
    During build/install extended_support directory will be created
    in the compiler package's include directory and vector_support.h
    will be copied there.
  * Create ../intel_newisaintrin.h (note that in this case you have to
    use "intel_" or "Intel_" prefix); add it into INTEL_HEADERS list
    in ../CMakeLists.txt as intel_newisaintrin.h;
    add "#include <intel_newisaintrin.h>" in ../immintrin.h.
    During build/install intel_newisaintrin.h will be copied directly
    into the compiler package's include directory.  When/If you open-source
    this header file, you will have to rename it and change its name
    inside ../immintrin.h.
