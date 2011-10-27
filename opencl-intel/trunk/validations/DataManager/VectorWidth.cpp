/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  VectorWidth.cpp

\*****************************************************************************/

#include "VectorWidth.h"

using namespace Validation;

bool VectorWidthWrapper::m_isStaticInit = false;

std::map<VectorWidth, VectorWidthWrapper::VectorWidthMetadata> VectorWidthWrapper::m_metaData;

void VectorWidthWrapper::initStatic() 
{
	// TODO: not thread safe
	m_metaData.clear();

	m_metaData[V1] = VectorWidthWrapper::VectorWidthMetadata(1, "v1");
	m_metaData[V2] = VectorWidthWrapper::VectorWidthMetadata(2, "v2");
	m_metaData[V3] = VectorWidthWrapper::VectorWidthMetadata(3, "v3");
	m_metaData[V4] = VectorWidthWrapper::VectorWidthMetadata(4, "v4");
	m_metaData[V8] = VectorWidthWrapper::VectorWidthMetadata(8, "v8");
	m_metaData[V16] = VectorWidthWrapper::VectorWidthMetadata(16, "v16");
	m_isStaticInit = true;
}

