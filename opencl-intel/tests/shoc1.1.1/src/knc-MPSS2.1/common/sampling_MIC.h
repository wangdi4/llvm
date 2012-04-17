/*COPYRIGHT**
 * -------------------------------------------------------------------------
 *               INTEL CORPORATION PROPRIETARY INFORMATION
 *  This software is supplied under the terms of the accompanying license
 *  agreement or nondisclosure agreement with Intel Corporation and may not
 *  be copied or disclosed except in accordance with the terms of that
 *  agreement.
 *        Copyright (c) 2007-2011 Intel Corporation. All Rights Reserved.
 * -------------------------------------------------------------------------
**COPYRIGHT*/
/*!
 * @file sampling_MIC.h
 * @brief Reduced API for sampling control in MIC usermode.
 *
 *  This header provides some small functionality to allow control from within
 *  a native MIC application and offload MIC application.  
 *  The control operation includes pause sampling, resume sampling.
 *  Once SEP tool running on the host side has configued the perfmon counters and/or 
 *  start/pause the sampling, users can use these APIs to pause/resume sampling in 
 *  an offload application or a native MIC application.
 *
 *  The APIs are defined same as in sampling.h for host sampling.
 *
 *  The APIs are simple enough that the full implementation is provided here in
 *  static inline functions.
 */
/*
 *  cvs_id[] = "$Id: sampling_MIC.h 27805 2008-07-18 17:04:40Z pzhang7 $"
 */
#ifndef _SAMPLING_MIC_H_INC_
#define _SAMPLING_MIC_H_INC_

#if defined(__cplusplus)
extern "C" {
#endif
#include "fcntl.h"
#include "sys/ioctl.h"
#include <string.h>
#include <unistd.h>

/* Usage note:
 * For an offload app, please define the OFFLOAD flag
 * For a native app, please comment out the OFFLOAD flag
 */
#define SEP_OFFLOAD

#if defined(SEP_OFFLOAD)
static int __declspec(target(mic)) VTPauseSampling(void);
static int __declspec(target(mic)) VTResumeSampling(void);
static char __declspec(target(mic)) sep_dev_name[] =  {"/dev/sep3_5/c"};
#else
static int VTPauseSampling(void);
static inline int VTResumeSampling(void);
static char sep_dev_name[] =  {"/dev/sep3_5/c"};
#endif

// return status
#define  SEP_CTL_SUCCESS   0
#define  SEP_CTL_FAILED    -1      // use errno to find a right error code


// These are defined in lwpmudrv_ioctl.h
#define SEP_API_IOC_MAGIC 	99
#define SEP_IOCTL_PAUSE      _IO (SEP_API_IOC_MAGIC, 31)
#define SEP_IOCTL_RESUME     _IO (SEP_API_IOC_MAGIC, 32)


static int
VTPauseSampling(void) 
{
    int handle = 0;
    int ret = SEP_CTL_FAILED;
    char sep_dev_name[] = {"/dev/sep3_5/c"}; 
    handle = open(sep_dev_name, O_RDWR);
    if (handle>0) { 
        ret = ioctl(handle, SEP_IOCTL_PAUSE);
        close(handle);
    }; 
    return ret;
}

static int
VTResumeSampling(void) 
{

    int handle = 0;
    int ret = SEP_CTL_FAILED;
    char sep_dev_name[] = {"/dev/sep3_5/c"}; 
    handle = open(sep_dev_name, O_RDWR);
    if (handle>0) { 
        ret = ioctl(handle, SEP_IOCTL_RESUME);
        close(handle);
    }; 
    return ret;
}

#if defined(__cplusplus)
}
#endif

#endif  // _SAMPLING_MIC_H_INC_
