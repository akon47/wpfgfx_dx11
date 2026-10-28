@echo off
rem 수동 컴파일용. 빌드에서는 ..\ShaderAssemblies.targets 가 같은 명령을 돌린다.
set FXCTOOL=fxc
%FXCTOOL% /T vs_4_0 Source\ShaderEffectsVS.fx /E VS /FoBinary\ShaderEffectVS.vsbin
%FXCTOOL% /T ps_4_0 Source\PassThroughShaderEffectPS.fx /E PS /FoBinary\PassThroughShaderEffectPS.psbin

@REM Blur Shaders
%FXCTOOL% /T ps_4_0 Source\HorizontalBlur.fx /E PS /FoBinary\HorizontalBlur.psbin
%FXCTOOL% /T ps_4_0 Source\VerticalBlur.fx /E PS /FoBinary\VerticalBlur.psbin
%FXCTOOL% /T ps_4_0 Source\HorizontalBlurMulti.fx /E PS /FoBinary\HorizontalBlurMulti.psbin
%FXCTOOL% /T ps_4_0 Source\VerticalBlurMulti.fx /E PS /FoBinary\VerticalBlurMulti.psbin

@REM Drop Shadow Shader
%FXCTOOL% /T ps_4_0 Source\DropShadow.fx /E PS /FoBinary\DropShadow.psbin

@REM Text Shaders
%FXCTOOL% /T ps_4_0 Source\TextShaders.fx /E PS_CTSB /FoBinary\TextCTSB.psbin
%FXCTOOL% /T ps_4_0 Source\TextShaders.fx /E PS_GSSB /FoBinary\TextGSSB.psbin
%FXCTOOL% /T ps_4_0 Source\TextShaders.fx /E PS_CTTB /FoBinary\TextCTTB.psbin
%FXCTOOL% /T ps_4_0 Source\TextShaders.fx /E PS_GSTB /FoBinary\TextGSTB.psbin
