CPU Level0 runtime
================================

This directory and its subdirectories contain source code for the CPU Level0
runtime.

================================

* the script `extract_empty_func` is used to parse the level_zero header file and create dummy implementation
for each API, the implemented API in ze_api.cpp will be excluded.

* the patch file ./enable_cpul0_compilation_in_ics.patch should be applied under xtoolsup/build/bin/ folder
 mannually so that we can compile it under ics with addtional option -bldcpul0. This operation will be removed
 when this patch is landed.
