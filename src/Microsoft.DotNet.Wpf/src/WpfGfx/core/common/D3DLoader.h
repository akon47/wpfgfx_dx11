// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+----------------------------------------------------------------------------
//
//  Abstract:        Maintains the primary DXGI factory reference and the
//                   display uniqueness query.
//
//-----------------------------------------------------------------------------

#pragma once

//+----------------------------------------------------------------------------
//
//  Class:
//      CD3DLoader
//
//  Synopsys:
//      Provides access to top level graphics objects.
//      Implementation is based on wrapping CDisplaySet object that
//      protects against unnecessary reloads when uniqueness change
//      notification turns to be irrelevant.
//
//-----------------------------------------------------------------------------
class CD3DLoader
{
public:

    //
    // Increase graphics module load count
    //

    static void GetLoadRef(
        );

    //
    // Decrement graphics module load count
    //
    // Note: Must be paired with GetLoadRef, but only called after all
    //       graphics interfaces derived from the factory have been released.
    //

    static void ReleaseLoadRef(
        );
};

//+----------------------------------------------------------------------------
//
//  Class:
//      CD3DModuleLoader
//
//  Synopsys:
//      Creates the DXGI factory and tracks its use count. d3d11.dll and
//      dxgi.dll are linked statically, so there is no module to load or
//      unload; the reference count only gates factory lifetime.
//
//-----------------------------------------------------------------------------
class CD3DModuleLoader
{
public:
    static HRESULT Startup();
    static void Shutdown();

private:
    // This class is not supposed to be used by anything
    // except CDisplaySet and CDisplayManager.
    friend class CDisplaySet;
    friend class CDisplayManager;

    //
    // Get the current uniqueness value
    //
    // Note: A valid load reference is not required.
    //

    static ULONG GetDisplayUniqueness();

    //
    // Create the top level DXGI factory and take a load reference.
    //

    static HRESULT CreateD3DObjects(
        __deref_out_ecount(1) IDXGIFactory1 **ppDXGIFactory
        );

    //
    // Release a load reference taken by CreateD3DObjects.
    //

    static void ReleaseD3DLoadRef(
        );

};
