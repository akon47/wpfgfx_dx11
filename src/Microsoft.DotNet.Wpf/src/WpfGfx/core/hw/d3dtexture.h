// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//-----------------------------------------------------------------------------
//
//  Description:
//      Contains D3D texture wrapper class declaration
//

class CD3DTexture : public CD3DResource
{
public:
    static void DetermineUsageAndLevels(
        __in_ecount(1) const CD3DDeviceLevel1 *pDevice,
        TextureMipMapLevel eMipMapLevel,
        UINT uTextureWidth,
        UINT uTextureHeight,
        __out_ecount(1) UINT *puBindFlags,
        __out_ecount(1) UINT *puMiscFlags,
        __out_ecount(1) UINT *puLevels
        );

    void GetTextureSize(
        __out_ecount(1) UINT *puWidth,
        __out_ecount(1) UINT *puHeight
        ) const;

    __out_ecount(1) ID3D11Texture2D *GetD3DTextureNoRef() const
    {
        Assert(IsValid());
        Assert(m_pD3DTexture);

        return m_pD3DTexture;
    }

    // Shader resource view over all levels. Created on first use, not AddRef'd.
    HRESULT GetShaderResourceViewNoRef(
        __deref_out_ecount(1) ID3D11ShaderResourceView **ppSRV
        );

    const D3D11_TEXTURE2D_DESC& D3DSurface0Desc() const { return m_sdLevel0; }

    __range(1,32) UINT Levels() const { return m_cLevels; }

    HRESULT GetD3DSurfaceLevel(
        UINT Level,
        __deref_out_ecount(1) CD3DSurface **ppSurfaceLevel
        );

    HRESULT UpdateMipmapLevels();

    // Lockable textures copy CPU side changes to the GPU texture here.
    virtual HRESULT EnsureGpuContents()
    {
        return S_OK;
    }

    override bool RequiresDelayedRelease() const
    {
        return true;
    }

protected:
    CD3DTexture();
    ~CD3DTexture();

    HRESULT Init(
        __inout_ecount(1) CD3DResourceManager *pResourceManager,
        __inout_ecount(1) ID3D11Texture2D *pD3DTexture
        );

    HRESULT InitResource(
        __inout_ecount(1) CD3DResourceManager *pResourceManager,
        __inout_ecount(1) ID3D11Texture2D *pD3DTexture
        );

protected:

    //
    // CD3DResource methods
    //

    // Should only be called by CD3DResourceManager (destructor is okay, too)
    void ReleaseD3DResources();

    ID3D11Texture2D *m_pD3DTexture;
    ID3D11ShaderResourceView *m_pSRV;
    D3D11_TEXTURE2D_DESC m_sdLevel0;

private:

    __field_range(1,32) DWORD m_cLevels;

    // Cache of surface levels
    __field_ecount_full_opt(m_cLevels) CD3DSurface **m_rgSurfaceLevel;
};
