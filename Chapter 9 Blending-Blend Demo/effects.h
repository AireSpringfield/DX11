//***************************************************************************************
// Effects.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Defines lightweight effect wrappers to group an effect and its variables.
// Also defines a static Effects class from which we can access all of our effects.
//***************************************************************************************

#ifndef EFFECTS_H
#define EFFECTS_H

#include"d3dutility.h"
#include"d3dx11effect.h"
#include"lighthelper.h"
#include<string>

class Effect
{
public:
	Effect(ID3D11Device* device, const std::wstring& filename);
	virtual ~Effect();

private:
	Effect(const Effect& rhs);
	Effect& operator=(const Effect& rhs);

protected:
	ID3DX11Effect* fx_ = nullptr;
};



class BasicEffect : public Effect
{
public:
	BasicEffect(ID3D11Device* device, const std::wstring& filename);
	~BasicEffect();

	inline void SetWorldViewProj(DirectX::CXMMATRIX M)                  { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	inline void SetWorld(DirectX::CXMMATRIX M)                          { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	inline void SetWorldInvTranspose(DirectX::CXMMATRIX M)              { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	inline void SetTexTransform(DirectX::CXMMATRIX M)                   { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	inline void SetEyePosW(const DirectX::XMFLOAT3& v)                  { EyePosW->SetRawValue(&v, 0, sizeof(DirectX::XMFLOAT3)); }
	inline void SetFogColor(const DirectX::FXMVECTOR v)                 { FogColor->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	inline void SetFogStart(float f)                           { FogStart->SetFloat(f); }
	inline void SetFogRange(float f)                           { FogRange->SetFloat(f); }
	inline void SetDirLights(const DirectionalLight* lights)   { DirLights->SetRawValue(lights, 0, 3*sizeof(DirectionalLight)); }
	inline void SetMaterial(const Material& mat)               { Mat->SetRawValue(&mat, 0, sizeof(Material)); }
	inline void SetDiffuseMap(ID3D11ShaderResourceView* tex)   { DiffuseMap->SetResource(tex); }

	ID3DX11EffectTechnique* Light1Tech = nullptr;
	ID3DX11EffectTechnique* Light2Tech = nullptr;
	ID3DX11EffectTechnique* Light3Tech = nullptr;

	ID3DX11EffectTechnique* Light0TexTech = nullptr;
	ID3DX11EffectTechnique* Light1TexTech = nullptr;
	ID3DX11EffectTechnique* Light2TexTech = nullptr;
	ID3DX11EffectTechnique* Light3TexTech = nullptr;

	ID3DX11EffectTechnique* Light0TexAlphaClipTech = nullptr;
	ID3DX11EffectTechnique* Light1TexAlphaClipTech = nullptr;
	ID3DX11EffectTechnique* Light2TexAlphaClipTech = nullptr;
	ID3DX11EffectTechnique* Light3TexAlphaClipTech = nullptr;

	ID3DX11EffectTechnique* Light1FogTech = nullptr;
	ID3DX11EffectTechnique* Light2FogTech = nullptr;
	ID3DX11EffectTechnique* Light3FogTech = nullptr;

	ID3DX11EffectTechnique* Light0TexFogTech = nullptr;
	ID3DX11EffectTechnique* Light1TexFogTech = nullptr;
	ID3DX11EffectTechnique* Light2TexFogTech = nullptr;
	ID3DX11EffectTechnique* Light3TexFogTech = nullptr;

	ID3DX11EffectTechnique* Light0TexAlphaClipFogTech = nullptr;
	ID3DX11EffectTechnique* Light1TexAlphaClipFogTech = nullptr;
	ID3DX11EffectTechnique* Light2TexAlphaClipFogTech = nullptr;
	ID3DX11EffectTechnique* Light3TexAlphaClipFogTech = nullptr;

	ID3DX11EffectMatrixVariable* WorldViewProj = nullptr;
	ID3DX11EffectMatrixVariable* World = nullptr;
	ID3DX11EffectMatrixVariable* WorldInvTranspose = nullptr;
	ID3DX11EffectMatrixVariable* TexTransform = nullptr;
	ID3DX11EffectVectorVariable* EyePosW = nullptr;
	ID3DX11EffectVectorVariable* FogColor = nullptr;
	ID3DX11EffectScalarVariable* FogStart = nullptr;
	ID3DX11EffectScalarVariable* FogRange = nullptr;
	ID3DX11EffectVariable* DirLights = nullptr;
	ID3DX11EffectVariable* Mat = nullptr;

	ID3DX11EffectShaderResourceVariable* DiffuseMap = nullptr;
};

class Effects
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static BasicEffect* BasicFX;
};


#endif // EFFECTS_H