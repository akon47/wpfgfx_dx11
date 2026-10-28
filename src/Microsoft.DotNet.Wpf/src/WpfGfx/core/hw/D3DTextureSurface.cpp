// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//-----------------------------------------------------------------------------
//
//  Description:
//      Contains CD3DTextureSurface implementation
//
//      Provides basic abstraction of a surface which is one mip level of a
//      D3D11 texture.
//

#include "precomp.hpp"

MtDefine(CD3DTextureSurface, MILRender, "CD3DTextureSurface");
MtDefine(D3DResource_TextureSurface, MILHwMetrics, "Approximate surface sizes");

//+------------------------------------------------------------------------
//
//  Member:    CD3DTextureSurface::Create
//
//  Synopsis:  Create a CD3DTextureSurface object to wrap a texture level
//
//-------------------------------------------------------------------------
HRESULT
CD3DTextureSurface::Create(
    __inout_ecount(1) CD3DResourceManager *pResourceManager,
    __inout_ecount(1) ID3D11Texture2D *pD3DTexture,
    UINT uLevel,
    __deref_out_ecount(1) CD3DSurface **ppSurface
    )
{
    HRESULT hr = S_OK;

    CD3DTextureSurface *pTextureSurface = NULL;

    //
    // Create the D3D surface wrapper
    //

    pTextureSurface = new CD3DTextureSurface(pD3DTexture, uLevel);
    IFCOOM(pTextureSurface);
    pTextureSurface->AddRef(); // CD3DTextureSurface::ctor sets ref count == 0

    //
    // Call init
    //

    IFC(pTextureSurface->Init(pResourceManager));

Cleanup:
    if (FAILED(hr))
    {
        ReleaseInterface(pTextureSurface);
    }

    *ppSurface = pTextureSurface;

    RRETURN(hr);
}

//+------------------------------------------------------------------------
//
//  Member:    CD3DTextureSurface::CD3DTextureSurface
//
//  Synopsis:  ctor
//
//-------------------------------------------------------------------------
CD3DTextureSurface::CD3DTextureSurface(
    __inout_ecount(1) ID3D11Texture2D * const pD3DTexture,
    UINT uLevel
    )
    : CD3DSurface(pD3DTexture, uLevel)
{
}

//+------------------------------------------------------------------------
//
//  Member:    CD3DTextureSurface::Init
//
//  Synopsis:  Inits the texture surface wrapper
//
//-------------------------------------------------------------------------
HRESULT
CD3DTextureSurface::Init(
    __inout_ecount(1) CD3DResourceManager *pResourceManager
    )
{
    HRESULT hr = S_OK;

    //
    // Read the description of the level
    //

    ReadDesc();

    //
    // Init the base class, use a zero size because texture wrapper will
    // already register with a size acounting for all levels
    //

    CD3DResource::Init(pResourceManager, 0);

    RRETURN(hr);
}

//+------------------------------------------------------------------------
//
//  Member:    CD3DTextureSurface::ReleaseD3DResources
//
//  Synopsis:  Release the surface.
//
//             This method may only be called by CD3DResourceManager because
//             there are various restrictions around when a call to
//             ReleaseD3DResources is okay.
//
//-------------------------------------------------------------------------
void
CD3DTextureSurface::ReleaseD3DResources()
{
    // This resource should have been marked invalid already or at least be out
    // of use.
    Assert(!m_fResourceValid || (m_cRef == 0));
    Assert(IsValid() == m_fResourceValid);

    ReleaseViews();

    // This context is protected so it is safe to release the D3D resource
    ReleaseInterface((*const_cast<ID3D11Texture2D **>(&m_pD3DTexture)));

    return;
}
