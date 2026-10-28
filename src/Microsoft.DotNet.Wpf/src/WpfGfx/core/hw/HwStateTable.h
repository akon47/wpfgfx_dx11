// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//
//  $TAG ENGR
//
//      $Module:    win_mil_graphics_d3d
//      $Keywords:
//
//  $Description:
//      Contains CStateTable declaration
//
//  $ENDTAG
//
//------------------------------------------------------------------------------

MtExtern(StateTableMemory);

//+-----------------------------------------------------------------------------
//
//  Class:
//      CStateTable
//
//  Synopsis:
//      Keeps track of unpredictable states
//
//      It provides functionality to track state objects which can be unknown.
//
//  Responsibilites:
//      - Track whether we know the value of each state
//      - Provide common functionality for testing if a state is set, retrieving
//        state values, invalidating state, and making sure we set only supported
//        values in Debug.
//
//  Not responsible for:
//      - Making the state setting calls to D3D
//      - Keeping references to AddRef'able objects
//
//  Inputs required:
//      - Number of states to track buffer
//
//  Implementation Details:
//      - Keeps an array of StateKnown/StateValue pairs.
//
//       [ StateKnown | StateValue ] [ StateKnown | StateValue ] [ StateKnown
//       ..]
//

template<class StateType>
class CStateTable
{
private:

    class CEntry;

public:

    CStateTable();
    ~CStateTable();

    HRESULT Init(
        __in_range(1, 999) DWORD dwStateTableSize
        );

    MIL_FORCEINLINE bool IsStateSet(
        DWORD dwStateNum,
        __in_ecount(1) const StateType &oStateValue
        ) const
    {
        #if DBG
        Assert(dwStateNum < m_uDbgNumStates);
        #endif

        const CEntry &oEntry = m_rgStateData[dwStateNum];

        return oEntry.IsStateSet(oStateValue);
    }

    HRESULT GetState(
        DWORD dwStateNum,
        __out_ecount(1) StateType *pStateValue
        ) const;

    HRESULT GetStateNoAddRef(
        DWORD dwStateNum,
        __out_ecount(1) StateType *pStateValue
        ) const;

    void SetToUnknown(
        DWORD dwStateNum
        );

    void SetAllToUnknown();

    void UpdateState(
        HRESULT hrStateChange,
        DWORD dwStateNum,
        __in_ecount(1) const StateType &oStateValue
        );

private:

    CEntry *m_rgStateData;  // The real state table
    UINT m_uNumStates;      // Number of states tracked

#if DBG
    UINT m_uDbgNumStates;   // Number of states tracked
#endif

};

//+-----------------------------------------------------------------------------
//
//  Class:
//      CStateTable::CEntry<StateType>
//
//  Synopsis:
//      Data and methods for tracking a particular D3D state
//

template<class StateType>
class CStateTable<StateType>::CEntry
{
public:

    DECLARE_METERHEAP_CLEAR(ProcessHeap, Mt(StateTableMemory));

    MIL_FORCEINLINE bool IsStateSet(
        __in_ecount(1) const StateType &oStateValue
        ) const
    {
        return IsKnown() && IsEqual(oStateValue);
    }

    MIL_FORCEINLINE bool IsEqual(__in_ecount(1) const StateType &oTestValue) const
    {
        if (m_oValue == oTestValue)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    MIL_FORCEINLINE void SetValue(
        __in_ecount(1) const StateType &oNewValue
        )
    {
        m_oValue = oNewValue;
    }

    MIL_FORCEINLINE void GetValue(
        __out_ecount(1) StateType &oOutValue
        ) const
    {
        oOutValue = m_oValue;
    }

    MIL_FORCEINLINE void GetValueNoAddRef(
        __out_ecount(1) StateType &oOutValue
        ) const
    {
        oOutValue = m_oValue;
    }

    bool IsKnown() const { return m_fKnown; }
    void SetKnown() { m_fKnown = true; }
    void SetUnknown() { m_fKnown = false; }

private:
    StateType m_oValue;     // When fKnown the state set in D3D

    bool m_fKnown;          // True if value of the state set in D3D is known
};

//+-----------------------------------------------------------------------------
//
//  Member:
//      CStateTable<StateType>::ctor
//
//  Synopsis:
//      Initialize the StateData to NULL.
//
//------------------------------------------------------------------------------
template<class StateType>
CStateTable<StateType>::CStateTable()
{
    m_rgStateData = NULL;
    m_uNumStates = 0;
#if DBG
    m_uDbgNumStates = 0;
#endif
};

//+-----------------------------------------------------------------------------
//
//  Member:
//      CStateTable<StateType>::dtor
//
//  Synopsis:
//      Release Memory.
//
//------------------------------------------------------------------------------
template<class StateType>
CStateTable<StateType>::~CStateTable()
{
    WPFFree(ProcessHeap, m_rgStateData);
};

//+-----------------------------------------------------------------------------
//
//  Member:
//      CStateTable<StateType>::Init
//
//  Synopsis:
//      Allocate the array and set its known values to FALSE.
//
//------------------------------------------------------------------------------
template<class StateType>
HRESULT
CStateTable<StateType>::Init(
    __in_range(1, 999) DWORD dwStateTableSize
    )
{
    HRESULT hr = S_OK;

    Assert(dwStateTableSize > 0);
    Assert(dwStateTableSize < 1000);

    m_uNumStates = dwStateTableSize;
#if DBG
    m_uDbgNumStates = dwStateTableSize;
#endif

    //
    // Allocate the actual state table
    //

    m_rgStateData = new CEntry[dwStateTableSize];
    IFCOOM(m_rgStateData);

    //
    // Because we set all the known values to FALSE, we don't need to set the
    // values of the states themselves.  DECLARE_METERHEAP_CLEAR actually
    // does the set to FALSE.
    //

    for (UINT dwStateNum = 0; dwStateNum < dwStateTableSize; dwStateNum++)
    {
        Assert(!m_rgStateData[dwStateNum].IsKnown());
    }

Cleanup:

    RRETURN(hr);

}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CStateTable<StateType>::GetState
//
//  Synopsis:
//      Returns the value of the State, or returns E_FAIL if the state value is
//      not known.
//
//------------------------------------------------------------------------------
template<class StateType>
HRESULT
CStateTable<StateType>::GetState(
    DWORD dwStateNum,
    __out_ecount(1) StateType *pStateValue
    ) const
{
    HRESULT hr = S_OK;

    #if DBG
    Assert(dwStateNum < m_uDbgNumStates);
    #endif
    Assert(pStateValue);

    CEntry const &oEntry = m_rgStateData[dwStateNum];

    if (!oEntry.IsKnown())
    {
        IFC(E_FAIL);
    }

    oEntry.GetValue(*pStateValue);

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CStateTable<StateType>::GetStateNoAddRef
//
//  Synopsis:
//      Returns the value of the State, or returns E_FAIL if the state value is
//      not known just like GetState.  However if the state is a ref counted
//      object no reference will be acquired.
//
//------------------------------------------------------------------------------
template<class StateType>
HRESULT
CStateTable<StateType>::GetStateNoAddRef(
    DWORD dwStateNum,
    __out_ecount(1) StateType *pStateValue
    ) const
{
    HRESULT hr = S_OK;

    #if DBG
    Assert(dwStateNum < m_uDbgNumStates);
    #endif
    Assert(pStateValue);

    CEntry const &oEntry = m_rgStateData[dwStateNum];

    if (!oEntry.IsKnown())
    {
        IFC(E_FAIL);
    }

    oEntry.GetValueNoAddRef(*pStateValue);

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CStateTable<StateType>::SetToUnknown
//
//  Synopsis:
//      Sets the state to Unknown.
//
//------------------------------------------------------------------------------
template<class StateType>
void
CStateTable<StateType>::SetToUnknown(
    DWORD dwStateNum
    )
{
    #if DBG
    Assert(dwStateNum < m_uDbgNumStates);
    #endif

    CEntry &oEntry = m_rgStateData[dwStateNum];

    oEntry.SetUnknown();
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CStateTable<StateType>::SetAllToUnknown
//
//  Synopsis:
//      Sets every state to Unknown.
//
//------------------------------------------------------------------------------
template<class StateType>
void
CStateTable<StateType>::SetAllToUnknown()
{
    for (UINT i = 0; i < m_uNumStates; i++)
    {
        m_rgStateData[i].SetUnknown();
    }
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CStateTable<StateType>::UpdateState
//
//  Synopsis:
//      This function should be called after the state is set.  It will update
//      the value of the state and whether it is known based on the error code
//      returned from the state setting operation.
//
//------------------------------------------------------------------------------
template<class StateType>
void
CStateTable<StateType>::UpdateState(
    HRESULT hrStateChange,
    DWORD dwStateNum,
    __in_ecount(1) const StateType &oStateValue
    )
{
    #if DBG
    Assert(dwStateNum < m_uDbgNumStates);
    #endif

    CEntry &oEntry = m_rgStateData[dwStateNum];

    //
    // If the state change operation failed, we set the state to unknown.
    //

    if (SUCCEEDED(hrStateChange))
    {
        oEntry.SetKnown();
        oEntry.SetValue(oStateValue);
    }
    else
    {
        oEntry.SetUnknown();
    }
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CStateTable<StateType>::CCEntry::GetValue specializations
//
//  Synopsis:
//      Save a new state value
//
//      NOTE: The HwStateTable does not keep references to AddRef'able
//            objects.  It assumes another reference is kept in another
//            location.  It will AddRef an object however, if it is
//            retrieved through GetValue.
//

template<>
MIL_FORCEINLINE void
CStateTable<ID3D11ShaderResourceView *>::CEntry::GetValue(
    __out_ecount(1) ID3D11ShaderResourceView *&oOutValue
    ) const
{
    oOutValue = m_oValue;

    if (m_oValue)
    {
        m_oValue->AddRef();
    }
}

template<>
MIL_FORCEINLINE void
CStateTable<ID3D11VertexShader *>::CEntry::GetValue(
    __out_ecount(1) ID3D11VertexShader *&oOutValue
    ) const
{
    oOutValue = m_oValue;

    if (m_oValue)
    {
        m_oValue->AddRef();
    }
}

template<>
MIL_FORCEINLINE void
CStateTable<ID3D11PixelShader *>::CEntry::GetValue(
    __out_ecount(1) ID3D11PixelShader *&oOutValue
    ) const
{
    oOutValue = m_oValue;

    if (m_oValue)
    {
        m_oValue->AddRef();
    }
}

template<>
MIL_FORCEINLINE void
CStateTable<ID3D11Buffer *>::CEntry::GetValue(
    __out_ecount(1) ID3D11Buffer *&oOutValue
    ) const
{
    oOutValue = m_oValue;

    if (m_oValue)
    {
        m_oValue->AddRef();
    }
}
