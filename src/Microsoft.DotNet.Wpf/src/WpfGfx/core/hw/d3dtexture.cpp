// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//-----------------------------------------------------------------------------
//
//  Description:
//      Contains CD3DTexture implementation
//
//      Provides basic abstraction of a D3D11 texture and tracks it as a D3D
//      resource.
//

#include "precomp.hpp"

MtDefine(TextureSurfaceCache, MILRender, "CD3DTexture surface cache array");

//+------------------------------------------------------------------------
//
//  Function:  CD3DTexture::CD3DTexture
//
//  Synopsis:  ctor
//
//-------------------------------------------------------------------------
CD3DTexture::CD3DTexture()
    : m_pD3DTexture(NULL),
      m_pSRV(NULL),
      m_rgSurfaceLevel(NULL)
{
    ZeroMemory(&m_sdLevel0, sizeof(m_sdLevel0));
}

//+------------------------------------------------------------------------
//
//  Function:  CD3DTexture::~CD3DTexture
//
//  Synopsis:  dtor
//
//-------------------------------------------------------------------------
CD3DTexture::~CD3DTexture()
{
    if (m_rgSurfaceLevel)
    {
        while (m_cLevels-- > 0)
        {
            ReleaseInterfaceNoNULL(m_rgSurfaceLevel[m_cLevels]);
        }
        WPFFree(ProcessHeap, m_rgSurfaceLevel);
    }

    ReleaseInterfaceNoNULL(m_pSRV);

    if (m_pD3DTexture != NULL)
    {
        // D26: intermediate render targets go back to the device's pool.
        Device().ReturnTextureToPool(m_pD3DTexture);
        m_pD3DTexture = NULL;
    }
}

//+------------------------------------------------------------------------
//
//  Function:  CD3DTexture::Init
//
//  Synopsis:  Inits the texture wrapper
//
//-------------------------------------------------------------------------
HRESULT
CD3DTexture::Init(
    __inout_ecount(1) CD3DResourceManager *pResourceManager,
    __inout_ecount(1) ID3D11Texture2D *pD3DTexture
    )
{
    HRESULT hr = S_OK;

    Assert(m_pD3DTexture == NULL);

    //
    // Get texture information
    //

    pD3DTexture->GetDesc(&m_sdLevel0);

    m_cLevels = m_sdLevel0.MipLevels;
    if (   (m_cLevels < 1)
        || (m_cLevels > 32))
    {
        IFC(E_FAIL);
    }

    //
    // Init the resouce base class
    //

    IFC(InitResource(pResourceManager, pD3DTexture));

Cleanup:
    RRETURN(hr);
}

//+------------------------------------------------------------------------
//
//  Function:  CD3DTexture::InitResource
//
//  Synopsis:  Inits the CD3DResouce base class
//
//-------------------------------------------------------------------------
HRESULT
CD3DTexture::InitResource(
    __inout_ecount(1) CD3DResourceManager *pResourceManager,
    __inout_ecount(1) ID3D11Texture2D *pD3DTexture
    )
{
    HRESULT hr = S_OK;
    UINT uPixelSize;
    UINT uResourceSize = 0;

    //
    // Compute the size of the resource
    //

    uPixelSize = D3DFormatSize(m_sdLevel0.Format);
    if (uPixelSize == 0)
    {
        IFC(D3DERR_WRONGTEXTUREFORMAT);
    }

    {
        UINT uWidth = m_sdLevel0.Width;
        UINT uHeight = m_sdLevel0.Height;

        for (UINT uLevel = 0; uLevel < m_cLevels; uLevel++)
        {
            // Accumulate each levels size
            uResourceSize += uWidth * uHeight * uPixelSize;

            uWidth = max(1u, uWidth / 2);
            uHeight = max(1u, uHeight / 2);
        }
    }

    //
    // Init the base class
    //

    CD3DResource::Init(pResourceManager, uResourceSize);

    //
    // Save the D3D texture reference
    //

    m_pD3DTexture = pD3DTexture;
    m_pD3DTexture->AddRef();

Cleanup:
    RRETURN(hr);
}

//+------------------------------------------------------------------------
//
//  Function:  CD3DTexture::ReleaseD3DResources
//
//  Synopsis:  Release the texture.
//
//-------------------------------------------------------------------------
void
CD3DTexture::ReleaseD3DResources()
{
    ReleaseInterface(m_pSRV);

    if (m_pD3DTexture != NULL)
    {
        // D26: intermediate render targets go back to the device's pool.
        // (The resource manager calls this under the device lock.) The
        // surface wrappers of this texture still hold references; they are
        // released with this object right after.
        ULONG cSurfaceRefs = 0;

        if (m_rgSurfaceLevel != NULL)
        {
            for (UINT i = 0; i < m_cLevels; i++)
            {
                if (m_rgSurfaceLevel[i] != NULL)
                {
                    cSurfaceRefs++;
                }
            }
        }

        Device().ReturnTextureToPool(m_pD3DTexture, 1 + cSurfaceRefs);
        m_pD3DTexture = NULL;
    }

    return;
}


//+------------------------------------------------------------------------
//
//  Function:  CD3DTexture::GetTextureSize
//
//  Synopsis:  Return the dimensions of the texture.
//
//-------------------------------------------------------------------------
void
CD3DTexture::GetTextureSize(
    __out_ecount(1) UINT *puWidth,
    __out_ecount(1) UINT *puHeight
    ) const
{
    Assert(IsValid());

    *puWidth = m_sdLevel0.Width;
    *puHeight = m_sdLevel0.Height;
}

//+------------------------------------------------------------------------
//
//  Function:  CD3DTexture::GetShaderResourceViewNoRef
//
//  Synopsis:  Get (and lazily create) the shader resource view.
//
//-------------------------------------------------------------------------
HRESULT
CD3DTexture::GetShaderResourceViewNoRef(
    __deref_out_ecount(1) ID3D11ShaderResourceView **ppSRV
    )
{
    HRESULT hr = S_OK;

    Assert(IsValid());
    Assert(m_sdLevel0.BindFlags & D3D11_BIND_SHADER_RESOURCE);

    if (m_pSRV == NULL)
    {
        IFC(MilTranslateD3D11Error(Device().GetD3DDeviceNoRef()->CreateShaderResourceView(
            m_pD3DTexture,
            NULL,
            &m_pSRV
            )));
    }

    *ppSRV = m_pSRV;

Cleanup:
    RRETURN(hr);
}

//+----------------------------------------------------------------------------
//
//  Member:    CD3DTexture::GetD3DSurfaceLevel
//
//  Synopsis:  Get a D3D surface wrapper for specified texture surface level
//

HRESULT
CD3DTexture::GetD3DSurfaceLevel(
    UINT Level,
    __deref_out_ecount(1) CD3DSurface ** const ppSurfaceLevel
    )
{
    HRESULT hr = S_OK;

    Assert(m_pD3DTexture);
    Assert(Level < m_cLevels);

    Device().Use(*this);

    if (!m_rgSurfaceLevel)
    {
        Assert(m_cLevels <= SIZE_T_MAX / sizeof(*m_rgSurfaceLevel));
        size_t const cbArray = sizeof(*m_rgSurfaceLevel) * m_cLevels;
        m_rgSurfaceLevel =
            WPFAllocTypeClear(CD3DSurface **, ProcessHeap, Mt(TextureSurfaceCache), cbArray);
        IFCOOM(m_rgSurfaceLevel);
    }

    if (!m_rgSurfaceLevel[Level])
    {
        //
        // Create the wrapper for the specified level
        //

        IFC(CD3DTextureSurface::Create(
            DYNCAST(CD3DResourceManager, m_pManager),
            m_pD3DTexture,
            Level,
            OUT &m_rgSurfaceLevel[Level]
            ));
    }

    *ppSurfaceLevel = m_rgSurfaceLevel[Level];
    (*ppSurfaceLevel)->AddRef();

Cleanup:
    RRETURN(hr);
}


//+------------------------------------------------------------------------
//
//  Function:  CD3DTexture::UpdateMipmapLevels
//
//  Synopsis:  Update nonzero mipmap levels of the texture based on the zero
//             level.
//
//-------------------------------------------------------------------------
HRESULT CD3DTexture::UpdateMipmapLevels()
{
    HRESULT hr = S_OK;

    if (m_cLevels > 1)
    {
        ID3D11ShaderResourceView *pSRV = NULL;

        Assert(m_sdLevel0.MiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS);

        IFC(GetShaderResourceViewNoRef(&pSRV));

        Device().GetD3DContextNoRef()->GenerateMips(pSRV);
    }

Cleanup:
    return hr;
}



//+----------------------------------------------------------------------------
//
//  Member:
//      CD3DTexture::DetermineUsageAndLevels
//
//  Synopsis:
//      Determines the bind flags, misc flags and levels for a texture that
//      might be mipmapped. Mipmaps are always generated by the GPU
//      (GenerateMips), which requires the texture to also be a render target.
//

void
CD3DTexture::DetermineUsageAndLevels(
    __in_ecount(1) const CD3DDeviceLevel1 *pDevice,
    TextureMipMapLevel eMipMapLevel,
    UINT uTextureWidth,
    UINT uTextureHeight,
    __out_ecount(1) UINT *puBindFlags,
    __out_ecount(1) UINT *puMiscFlags,
    __out_ecount(1) UINT *puLevels
    )
{
    UNREFERENCED_PARAMETER(pDevice);

    if (eMipMapLevel == TMML_One)
    {
        *puBindFlags = D3D11_BIND_SHADER_RESOURCE;
        *puMiscFlags = 0;
        *puLevels = 1;
    }
    else
    {
        Assert(eMipMapLevel == TMML_All);

        UINT uMaxSize = max(uTextureWidth, uTextureHeight);

        *puBindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        *puMiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
        // Request levels all the way down to 1x1
        *puLevels = Log2(uMaxSize) + 1;
    }
}
