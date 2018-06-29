// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "d3d11_mapper.h"

namespace Intel { namespace OpenCL { namespace Framework
{

ID3D11Buffer* D3d11BufferMapper::CreateResource(const D3D11_BUFFER_DESC& desc)
{
    ID3D11Buffer* pBuffer;
    const HRESULT res = GetDevice().CreateBuffer(&desc, nullptr, &pBuffer);
    if (E_OUTOFMEMORY == res)
    {
        LOG_ERROR(TEXT("There is insufficient memory to create the buffer"));
        return nullptr;
    }
    assert(S_OK == res);
    return pBuffer;
}

ID3D11Texture2D* D3d11Texture2DMapper::CreateResource(const D3D11_TEXTURE2D_DESC& desc)
{
    ID3D11Texture2D* pTexture2D;
	D3D11_TEXTURE2D_DESC newDesc = desc;
	
    // 0 MipLevels is used to generate a full set of sub-textures
    assert(desc.MipLevels > 0);
	const UINT uiSubresourceMipLevel = m_uiSubresource % desc.MipLevels;

	newDesc.Width >>= uiSubresourceMipLevel;
	if (0 == newDesc.Width)
	{
		newDesc.Width = 1;
	}
	newDesc.Height >>= uiSubresourceMipLevel;
	if (0 == newDesc.Height)
	{
		newDesc.Height = 1;
	}
	newDesc.MipLevels = 1;

    const HRESULT res = GetDevice().CreateTexture2D(&newDesc, nullptr, &pTexture2D);
    if (E_OUTOFMEMORY == res)
    {
        LOG_ERROR(TEXT("There is insufficient memory to create the 2D texture"));
        return nullptr;
    }
    assert(S_OK == res);
    return pTexture2D;
}

ID3D11Texture3D* D3d11Texture3DMapper::CreateResource(const D3D11_TEXTURE3D_DESC& desc)
{
    ID3D11Texture3D* pTexture3D;
	D3D11_TEXTURE3D_DESC newDesc = desc;

	newDesc.Width >>= m_uiSubresource;
	if (0 == newDesc.Width)
	{
		newDesc.Width = 1;
	}
	newDesc.Height >>= m_uiSubresource;
	if (0 == newDesc.Height)
	{
		newDesc.Height = 1;
	}
	newDesc.Depth >>= m_uiSubresource;
	if (0 == newDesc.Depth)
	{
		newDesc.Depth = 1;
	}
	newDesc.MipLevels = 1;
    
	const HRESULT res = GetDevice().CreateTexture3D(&newDesc, nullptr, &pTexture3D);
    if (E_OUTOFMEMORY == res)
    {
        LOG_ERROR(TEXT("There is insufficient memory to create the 3D texture"));
        return nullptr;
    }
    assert(S_OK == res);
    return pTexture3D;
}

}}}
