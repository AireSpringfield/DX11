#ifndef EFFECTS_H
#define EFFECTS_H

#include"d3dutility.h"
#include"d3dx11effect.h"
#include"lighthelper.h"
#include<string>
#include<DirectXMath.h>
using namespace DirectX; 

class Effect {
public:
	Effect(ID3D11Device *device, std::wstring compiledFileName);
	virtual ~Effect();


private:
	Effect(const Effect& rhs);
	Effect& operator=(const Effect& rhs);
protected:
	ID3DX11Effect *fx_ = nullptr;

};

class BasicEffect : public Effect{
public:
	BasicEffect(ID3D11Device *device, std::wstring compiledFileName);
	~BasicEffect();

public:

	ID3DX11EffectTechnique *light1Tech = nullptr;
	ID3DX11EffectTechnique *light2Tech = nullptr;
	ID3DX11EffectTechnique *light3Tech = nullptr;
	ID3DX11EffectTechnique *light0TexTech = nullptr;
	ID3DX11EffectTechnique *light1TexTech = nullptr;
	ID3DX11EffectTechnique *light2TexTech = nullptr;
	ID3DX11EffectTechnique *light3TexTech = nullptr;


	inline void SetWorldViewProj(CXMMATRIX M) { worldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	inline void SetWorld(CXMMATRIX M) { world->SetMatrix(reinterpret_cast<const float*>(&M)); }
	inline void SetWorldInvTranspose(CXMMATRIX M) { worldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	inline void SetTexTransform(CXMMATRIX M) { texTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	inline void SetEyePosW(const XMFLOAT3& v) { eyePosWorld->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	inline void SetDirLights(const DirectionalLight* lights) { dirLights->SetRawValue(lights, 0, 3 * sizeof(DirectionalLight)); }
	inline void SetMaterial(const Material& mat) { material->SetRawValue(&mat, 0, sizeof(Material)); }
	inline void SetDiffuseMap(ID3D11ShaderResourceView* tex) { diffuseMap->SetResource(tex); };



	ID3DX11EffectMatrixVariable *worldViewProj = nullptr;
	ID3DX11EffectMatrixVariable *world = nullptr;
	ID3DX11EffectMatrixVariable *worldInvTranspose = nullptr;
	ID3DX11EffectMatrixVariable *texTransform = nullptr;
	ID3DX11EffectVariable *material = nullptr;
	ID3DX11EffectVariable *dirLights = nullptr;
	ID3DX11EffectVectorVariable *eyePosWorld = nullptr;

	ID3DX11EffectShaderResourceVariable *diffuseMap = nullptr;

};


class Effects {
public:
	static void InitAll(ID3D11Device *device);
	static void	DestoryAll();

	static BasicEffect *basicFX;
};

#endif 