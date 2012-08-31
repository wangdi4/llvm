/*********************************************************************************************
 * Copyright © 2010, Intel Corporation
 * Subject to the terms and conditions of the Master Development License
 * Agreement between Intel and Apple dated August 26, 2005; under the Intel
 * CPU Vectorizer for OpenCL Category 2 PA License dated January 2010; and RS-NDA #58744
 *********************************************************************************************/
#ifndef __LLVM_ENV_ADAPT__H__
#define __LLVM_ENV_ADAPT__H__

#define VOLCANO_ENV 1
#define APPLE_ENV 0



#if APPLE_ENV
    #if !defined(MAX_LOOP_SIZE)
    Error! Target max loop size not defined!
    #endif
#endif






#endif // __LLVM_ENV_ADAPT__H__