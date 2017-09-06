#include"vertex.h"
#include"effects.h"

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::desc[2] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};


ID3D11InputLayout *InputLayouts::posNormal = nullptr;

void InputLayouts::InitAll(ID3D11Device *device) {

	D3DX11_PASS_DESC passDesc;
	Effects::basicFX->light1Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayoutDesc::desc, 2, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize,
		&posNormal));

}
void InputLayouts::DestroyAll() {
	ReleaseCOM(posNormal);
}