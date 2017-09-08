#include"effects.h"
#include<fstream>
#include<vector>

Effect::Effect(ID3D11Device *device, std::wstring compiledFileName) {
	
	std::ifstream fin(compiledFileName, std::ios::binary);
	if (fin.fail()) throw("Failed to open the compiled Effect file!");

	fin.seekg(0, std::ios_base::end);
	int size = fin.tellg();

	fin.seekg(0, std::ios_base::beg);

	std::vector<char> compiledShader(size);

	fin.read(&compiledShader[0], size);
	fin.close();

	HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size, 0, device, &fx_));

}

Effect::~Effect() {
	ReleaseCOM(fx_);
}




Effect::Effect(const Effect& rhs) {
	*this = rhs;
}

Effect& Effect::operator=(const Effect& rhs) {
	*this = rhs;
	return *this;
}

BasicEffect::BasicEffect(ID3D11Device *device, std::wstring compiledFileName)
	:Effect(device, compiledFileName) {

	light1Tech = fx_->GetTechniqueByName("Light1");
	light2Tech = fx_->GetTechniqueByName("Light2");
	light3Tech = fx_->GetTechniqueByName("Light3");

	light0TexTech = fx_->GetTechniqueByName("Light0Tex");
	light1TexTech = fx_->GetTechniqueByName("Light1Tex");
	light2TexTech = fx_->GetTechniqueByName("Light2Tex");
	light3TexTech = fx_->GetTechniqueByName("Light3Tex");

	worldViewProj= fx_->GetVariableByName("gWorldViewProj")->AsMatrix();
	world = fx_->GetVariableByName("gWorld")->AsMatrix();
	worldInvTranspose = fx_->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	texTransform = fx_->GetVariableByName("gTexTransform")->AsMatrix();
	eyePosWorld = fx_->GetVariableByName("gEyePosWorld")->AsVector();
	dirLights = fx_->GetVariableByName("gDirLights");
	material = fx_->GetVariableByName("gMaterial");
	
	diffuseMap = fx_->GetVariableByName("gDiffuseMap")->AsShaderResource();

	// For exercise 2.
	mipMap = fx_->GetVariableByName("gMipMap")->AsShaderResource();

	// For exercise 3.
	flare = fx_->GetVariableByName("gFlare")->AsShaderResource();
	flareAlpha = fx_->GetVariableByName("gFlareAlpha")->AsShaderResource();

	// For exercise 5.
	fire = fx_->GetVariableByName("gFire")->AsShaderResource();
}

BasicEffect::~BasicEffect() {
	ReleaseCOM(fx_);
}

BasicEffect *Effects::basicFX = nullptr;

void Effects::InitAll(ID3D11Device *device) {
	basicFX = new BasicEffect(device, L"FX/basic.fxo");
}

void Effects::DestroyAll() {
	SafeDelete(basicFX);
}