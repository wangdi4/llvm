/////////////////////////////////////////////////////////////////////////
// cl_cpu_detect.h
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2008 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related
// to the source code ("Material") are owned by Intel Corporation or its
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and
// proprietary and confidential information of Intel Corporation and its
// suppliers and licensors, and is protected by worldwide copyright and trade
// secret laws and treaty provisions. No part of the Material may be used, copied,
// reproduced, modified, published, uploaded, posted, transmitted, distributed,
// or disclosed in any way without Intel’s prior express written permission.
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel or
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors
// in any way.
/////////////////////////////////////////////////////////////////////////
#pragma once

#include "cl_types.h"
#include "cl_env.h"

namespace Intel { namespace OpenCL { namespace Utils {

	// IsCPUSupported - Check that the current CPU is alligned with the required HW platform
	// returncs CL_SUCCESS if the cpu supported
	// returns CL_ERR_CPU_NOT_SUPPORTED otherwise
	cl_err_code IsCPUSupported();

    // CPU Features enumeration
    enum ECPUFeatureSupport
    {
        CFS_NONE    = 0x0000,
        CFS_SSE2    = 0x0001,
        CFS_SSE3    = 0x0002,
        CFS_SSSE3   = 0x0004,
        CFS_SSE41   = 0x0008,
        CFS_SSE42   = 0x0010,
        CFS_AVX10   = 0x0020,
        CFS_AVX20   = 0x0040,
        CFS_FMA     = 0x0080
    };

    // Processor microarchitecture
    enum EMicroArchitecture
    {
        MA_ALL,
        MA_YONAH,
        MA_MEROM,
        MA_PENRYN,
        MA_NEHALEM,
        MA_WESTMERE,
        MA_SANDYBRIDGE,
        MA_IVYBRIDGE,
        MA_HASWELL,
        MA_BROADWELL,
        MA_SKYLAKE,
        MA_CANNONLAKE
    };

    // Processor brand family
    enum ECPUBrandFamily
    {
        BRAND_UNKNOWN,
        BRAND_INTEL_CORE,
        BRAND_INTEL_ATOM,
        BRAND_INTEL_PENTIUM,
        BRAND_INTEL_CELERON,
        BRAND_INTEL_XEON
    };

	// CPU detection class (singleton)
    class CPUDetect
    {
    public:

		static CPUDetect * GetInstance();

		bool IsGenuineIntel();

        bool isBroadwell();
        bool isBroxton();
        bool isSkylake();
        bool isKabylake();

		bool IsMicroArchitecture(EMicroArchitecture microArchitecture);

		bool IsFeatureSupported(ECPUFeatureSupport featureType);

		unsigned char	GetStepping() { return m_ucStepping; }
		unsigned char	GetModel() { return m_ucModel; }
		unsigned char	GetFamily() { return m_ucFamily; }
		unsigned char	GetType() { return m_ucType; }
		const char *	GetCPUString() { return m_szCPUString; }
		const char *	GetCPUBrandString() { return m_szCPUBrandString; }
		unsigned int	GetCPUFeatureSupport() { return m_uiCPUFeatures; }
		unsigned int	GetCoreCount() { return m_uiCoreCount; }
		ECPUBrandFamily	GetCPUBrandFamily() { return m_eCPUBrand; }

	private:
		CPUDetect& operator=(const CPUDetect&);
		CPUDetect(const CPUDetect&);
		CPUDetect();
		~CPUDetect();

        bool			m_bBypassCPUDetect;
        bool			m_bIsGenuineIntel;
        unsigned char	m_ucStepping;
        unsigned char	m_ucModel;
        unsigned char	m_ucExtendedModel;
        unsigned char	m_ucFamily;
        unsigned char	m_ucType;
        char *			m_szCPUString;
        char *			m_szCPUBrandString;
        unsigned int	m_uiCPUFeatures;
		unsigned int	m_uiCoreCount;
        unsigned short  m_i16ProcessorSignature;
        ECPUBrandFamily m_eCPUBrand;

		void			GetCPUInfo();
		bool			ShouldBypassCPUCheck();

    };

}}}
