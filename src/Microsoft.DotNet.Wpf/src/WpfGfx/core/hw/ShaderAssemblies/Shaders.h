// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//
//  셰이더 리소스 ID. ShaderAssemblies.targets가 fxc로 만든 바이트코드를
//  ResourceFileTemplate\EmbeddedShaders.RC가 이 ID로 RCDATA에 넣는다.
//
//------------------------------------------------------------------------------

#define VS_ShaderEffects 901
#define PS_PassThroughShaderEffect 902

#define PS_BlurH 903
#define PS_BlurV 904
#define PS_BlurHMulti 905
#define PS_BlurVMulti 906

#define PS_DropShadow 907

// 텍스트 렌더링 (ClearType/GreyScale x SolidBrush/TextureBrush)
#define PS_Text_CTSB 910
#define PS_Text_GSSB 911
#define PS_Text_CTTB 912
#define PS_Text_GSTB 913
