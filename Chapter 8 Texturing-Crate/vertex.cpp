#include"vertex.h"
#include"effects.h"

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::desc[3] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
};


ID3D11InputLayout *InputLayouts::basic32 = nullptr;

void InputLayouts::InitAll(ID3D11Device *device) {

	D3DX11_PASS_DESC passDesc;
	Effects::basicFX->light1Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayoutDesc::desc, 3, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize,
		&basic32));

}
void InputLayouts::DestroyAll() {
	ReleaseCOM(basic32);
}