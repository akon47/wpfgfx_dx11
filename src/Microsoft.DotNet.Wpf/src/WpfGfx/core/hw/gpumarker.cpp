// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//

//
//  $TAG ENGR

//      $Module:    win_mil_graphics_d3d
//      $Keywords:
//
//  $Description:
//      GPUMarker class used to monitor GPU process of rendering
//
//  $ENDTAG
//
//------------------------------------------------------------------------------

#include "precomp.hpp"

MtDefine(CGPUMarker, MILRender, "CGPUMarker");

//+-----------------------------------------------------------------------------
//
//  Member:
//      CGPUMarker::CGPUMarker
//
//  Synopsis:
//      Constructor. Takes ownership of the query reference.
//
//------------------------------------------------------------------------------
CGPUMarker::CGPUMarker(
    __in_ecount(1) ID3D11Query *pQuery,
    __in_ecount(1) ID3D11DeviceContext *pContext,
    ULONGLONG ullMarkerId
    ) : m_pQuery(pQuery), m_pContext(pContext)
{
    m_pContext->AddRef();
    Reset(ullMarkerId);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CGPUMarker::~CGPUMarker
//
//  Synopsis:
//      Destructor
//
//------------------------------------------------------------------------------
CGPUMarker::~CGPUMarker()
{
    ReleaseInterface(m_pQuery);
    ReleaseInterface(m_pContext);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CGPUMarker::Reset
//
//  Synopsis:
//      Resets the marker for reuse
//
//------------------------------------------------------------------------------
void CGPUMarker::Reset(ULONGLONG ullMarkerId)
{
    m_fIssued = FALSE;
    m_fConsumed = FALSE;
    m_ullMarkerId = ullMarkerId;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CGPUMarker::InsertIntoCommandStream
//
//  Synopsis:
//      Insert the marker into the command stream
//
//  Returns:
//      S_OK if succeeds
//
//------------------------------------------------------------------------------
HRESULT CGPUMarker::InsertIntoCommandStream()
{
    HRESULT hr = S_OK;

    if (!m_pQuery)
    {
        IFC(E_FAIL);
    }

    m_pContext->End(m_pQuery);
    m_fIssued = TRUE;

  Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CGPUMarker::CheckStatus
//
//  Synopsis:
//      Checks if a marker has been processed by the GPU
//
//  Returns:
//      *pfConsumed will be true if the marker was consumed, false otherwise.
//
//------------------------------------------------------------------------------
HRESULT CGPUMarker::CheckStatus(
    BOOL fFlush,
    __out_ecount(1) bool *pfConsumed
    )
{
    HRESULT hr = S_OK;

    *pfConsumed = false;

    if (m_fIssued)
    {
        if (!m_fConsumed)
        {
            UINT uFlags = (fFlush ? 0 : D3D11_ASYNC_GETDATA_DONOTFLUSH);
            BOOL fDone = FALSE;

            MIL_THR(m_pContext->GetData(m_pQuery, &fDone, sizeof(fDone), uFlags));
            m_fConsumed = (hr == S_OK && fDone);

            if (hr == S_FALSE)
            {
                hr = S_OK;
            }

            hr = MilTranslateD3D11Error(hr);
        }

        *pfConsumed = (m_fConsumed != 0);
    }

    RRETURN(hr);
}
