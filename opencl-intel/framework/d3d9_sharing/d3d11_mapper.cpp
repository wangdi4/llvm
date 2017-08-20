// Copyright (c) 2006-2012 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

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
