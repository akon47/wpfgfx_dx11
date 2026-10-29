// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#include <precomp.hpp>
#include "WpfGfxSwitches.h"

// Static storage for WpfGfxSwitches
bool WpfGfxSwitches::g_fWpfGfxBoundsCheckProtectionDisabled = false;

//
// D37: .NET Framework 4.8.x's PresentationCore declares this as returning int
// and passes the result to HRESULT.Check; with a void return it reads a stale
// register and MediaSystem.Startup throws. .NET 8/10 declare it void and
// ignore the result, so returning S_OK leaves their contract unchanged.
//
HRESULT WINAPI
WpfGfx_SetDisableBoundsCheckProtection(BOOL fDisable)
{
    WpfGfxSwitches::SetWpfGfxBoundsCheckProtectionDisabled(!!fDisable);
    return S_OK;
}
