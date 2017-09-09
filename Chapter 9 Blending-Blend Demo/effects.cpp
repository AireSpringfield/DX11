//***************************************************************************************
// Effects.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include"Effects.h"
#include<fstream>
#include<vector>

#pragma region Effect
Effect::Effect(ID3D11Device* device, const std::wstring& filename)

{
	std::ifstream fin(filename, std::ios::binary);
	
	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);

	fin.read(&compiledShader[0], size);
	fin.close();
	
	HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size, 
		0, device, &fx_));
}

Effect::~Effect()
{
	ReleaseCOM(fx_);
}
#pragma endregion

#pragma region BasicEffect
BasicEffect::BasicEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	Light1Tech    = fx_->GetTechniqueByName("Light1");
	Light2Tech    = fx_->GetTechniqueByName("Light2");
	Light3Tech    = fx_->GetTechniqueByName("Light3");

	Light0TexTech = fx_->GetTechniqueByName("Light0Tex");
	Light1TexTech = fx_->GetTechniqueByName("Light1Tex");
	Light2TexTech = fx_->GetTechniqueByName("Light2Tex");
	Light3TexTech = fx_->GetTechniqueByName("Light3Tex");

	Light0TexAlphaClipTech = fx_->GetTechniqueByName("Light0TexAlphaClip");
	Light1TexAlphaClipTech = fx_->GetTechniqueByName("Light1TexAlphaClip");
	Light2TexAlphaClipTech = fx_->GetTechniqueByName("Light2TexAlphaClip");
	Light3TexAlphaClipTech = fx_->GetTechniqueByName("Light3TexAlphaClip");

	Light1FogTech    = fx_->GetTechniqueByName("Light1Fog");
	Light2FogTech    = fx_->GetTechniqueByName("Light2Fog");
	Light3FogTech    = fx_->GetTechniqueByName("Light3Fog");

	Light0TexFogTech = fx_->GetTechniqueByName("Light0TexFog");
	Light1TexFogTech = fx_->GetTechniqueByName("Light1TexFog");
	Light2TexFogTech = fx_->GetTechniqueByName("Light2TexFog");
	Light3TexFogTech = fx_->GetTechniqueByName("Light3TexFog");

	Light0TexAlphaClipFogTech = fx_->GetTechniqueByName("Light0TexAlphaClipFog");
	Light1TexAlphaClipFogTech = fx_->GetTechniqueByName("Light1TexAlphaClipFog");
	Light2TexAlphaClipFogTech = fx_->GetTechniqueByName("Light2TexAlphaClipFog");
	Light3TexAlphaClipFogTech = fx_->GetTechniqueByName("Light3TexAlphaClipFog");

	WorldViewProj     = fx_->GetVariableByName("gWorldViewProj")->AsMatrix();
	World             = fx_->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose = fx_->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	TexTransform      = fx_->GetVariableByName("gTexTransform")->AsMatrix();
	EyePosW           = fx_->GetVariableByName("gEyePosW")->AsVector();
	FogColor          = fx_->GetVariableByName("gFogColor")->AsVector();
	FogStart          = fx_->GetVariableByName("gFogStart")->AsScalar();
	FogRange          = fx_->GetVariableByName("gFogRange")->AsScalar();
	DirLights         = fx_->GetVariableByName("gDirLights");
	Mat               = fx_->GetVariableByName("gMaterial");
	DiffuseMap        = fx_->GetVariableByName("gDiffuseMap")->AsShaderResource();
}

BasicEffect::~BasicEffect()
{
}
#pragma endregion

#pragma region Effects

BasicEffect* Effects::BasicFX = 0;

void Effects::InitAll(ID3D11Device* device)
{
	BasicFX = new BasicEffect(device, L"FX/basic.fxo");
}

void Effects::DestroyAll()
{
	SafeDelete(BasicFX);
}
#pragma endregion