
#include"d3dapp.h"
#include"d3dutility.h"
#include"lighthelper.h"
#include"mathhelper.h"
#include"effects.h"
#include"vertex.h"
#include"geometrygenerator.h"

using namespace DirectX;

class CrateApp : public D3DApp {
public:
	CrateApp(HINSTANCE hInstance);
	~CrateApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

	void OnMouseWheel(WPARAM wParam, LPARAM lParam);

private:
	void BuildGeometryBuffers();


private:
	ID3D11Buffer *boxVB_ = nullptr;
	ID3D11Buffer *boxIB_ = nullptr;
	int boxVertexOffset_ = 0;
	UINT boxIndexOffset_ = 0;
	UINT boxIndexCnt_ = 0;

	ID3D11ShaderResourceView *diffuseMapSRV_ = nullptr;

	DirectionalLight dirLights_[3];
	Material boxMaterial_;


	XMFLOAT4X4 texTransform_ = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 boxWorld_ = MathHelper::XMFloat4x4Identity();

	XMFLOAT4X4 view_ = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 proj_ = MathHelper::XMFloat4x4Identity();

	XMFLOAT3 eyePosInWorld_ = XMFLOAT3(0.0f, 0.0f, 0.0f);

	float theta_ = 1.5f*MathHelper::Pi;
	float phi_ = 0.5f*MathHelper::Pi;
	float radius_ = 1.0f;
	POINT lastMousePos_ = { 0,0 };

};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	CrateApp app(hInstance);

	if (!app.Init())
		return 0;

	return app.Run();
}

CrateApp::CrateApp(HINSTANCE hInstance)
: D3DApp(hInstance){
	main_wnd_caption_ = L"Crate Demo";

	dirLights_[0].ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	dirLights_[0].diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	dirLights_[0].specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
	dirLights_[0].direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	dirLights_[1].ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	dirLights_[1].diffuse = XMFLOAT4(1.4f, 1.4f, 1.4f, 1.0f);
	dirLights_[1].specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
	dirLights_[1].direction = XMFLOAT3(-0.707f, 0.0f, 0.707f);

	boxMaterial_.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	boxMaterial_.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	boxMaterial_.specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);

}


CrateApp::~CrateApp() {
	ReleaseCOM(boxVB_);
	ReleaseCOM(boxIB_);
	ReleaseCOM(diffuseMapSRV_);

	Effects::DestoryAll();
	InputLayouts::DestroyAll();
}

bool CrateApp::Init() {
	if (!D3DApp::Init())
		return false;

	Effects::InitAll(device_);
	InputLayouts::InitAll(device_);

	CreateDDSShaderResourceViewFromFile(device_, L"Textures/WoodCrate01.dds", &diffuseMapSRV_);


	BuildGeometryBuffers();

	return true;

}


void CrateApp::OnResize() {
	D3DApp::OnResize();

	XMMATRIX proj = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&proj_, proj);
}

void CrateApp::UpdateScene(float dt) {
	float x = radius_*sinf(phi_)*cosf(theta_);
	float z = radius_*sinf(phi_)*sinf(theta_);
	float y = radius_*cosf(phi_);

	
	// Create the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&view_, view);

	eyePosInWorld_ = XMFLOAT3(x, y, z);


}

void CrateApp::DrawScene() {
	immediate_context_->ClearRenderTargetView(render_target_view_, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	immediate_context_->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	immediate_context_->IASetInputLayout(InputLayouts::basic32);
	immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT strides[1] = { sizeof(Vertex::Basic32) };
	UINT offsets[1] = { 0 };

	XMMATRIX view = XMLoadFloat4x4(&view_);
	XMMATRIX proj = XMLoadFloat4x4(&proj_);
	XMMATRIX viewProj = view*proj;

	Effects::basicFX->SetDirLights(dirLights_);
	Effects::basicFX->SetEyePosW(eyePosInWorld_);

	ID3DX11EffectTechnique *activeTech = Effects::basicFX->light2TexTech;

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; ++p) {
		immediate_context_->IASetVertexBuffers(0, 1, &boxVB_, strides, offsets);
		immediate_context_->IASetIndexBuffer(boxIB_, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&boxWorld_);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX view = XMLoadFloat4x4(&view_);
		XMMATRIX proj = XMLoadFloat4x4(&proj_);
		XMMATRIX worldViewProj = world*view*proj;
		XMMATRIX texTransform = XMLoadFloat4x4(&texTransform_);


		Effects::basicFX->SetWorld(world);
		Effects::basicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::basicFX->SetWorldViewProj(worldViewProj);
		Effects::basicFX->SetTexTransform(texTransform);

		Effects::basicFX->SetEyePosW(eyePosInWorld_);

		Effects::basicFX->SetDirLights(dirLights_);
		Effects::basicFX->SetMaterial(boxMaterial_);

		Effects::basicFX->SetDiffuseMap(diffuseMapSRV_);
		

		activeTech->GetPassByIndex(p)->Apply(0, immediate_context_);
		immediate_context_->DrawIndexed(boxIndexCnt_, boxIndexOffset_, boxVertexOffset_);
	}

	HR(swap_chain_->Present(0, 0));
}

void CrateApp::OnMouseWheel(WPARAM wParam, LPARAM lParam) {
	short delta = GET_WHEEL_DELTA_WPARAM(wParam);
	int inc = -delta / WHEEL_DELTA;
	radius_ += inc*5.0f;
	radius_ = MathHelper::Clamp(radius_, 0.5f, 100.0f);
}

void CrateApp::OnMouseUp(WPARAM btnState, int x, int y) {
	ReleaseCapture();
}
void CrateApp::OnMouseDown(WPARAM btnState, int x, int y) {
	lastMousePos_.x = x;
	lastMousePos_.y = y;
	SetCapture(main_wnd_);
}
void CrateApp::OnMouseMove(WPARAM btnState, int x, int y) {
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - lastMousePos_.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - lastMousePos_.y));

		// Update angles based on input to orbit camera around box.
		theta_ += dx;
		phi_ += dy;

		// Restrict the angle mPhi.
		phi_ = MathHelper::Clamp(phi_, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.01 unit in the scene.
		float dx = 0.01f*static_cast<float>(x - lastMousePos_.x);
		float dy = 0.01f*static_cast<float>(y - lastMousePos_.y);

		// Update the camera radius based on input.
		radius_ += dx - dy;

		// Restrict the radius.
		radius_ = MathHelper::Clamp(radius_, 3.0f, 200.0f);
	}



	lastMousePos_.x = x;
	lastMousePos_.y = y;
}

void CrateApp::BuildGeometryBuffers() {
	GeometryGenerator::MeshData box;

	GeometryGenerator geoGen;

	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);

	boxVertexOffset_ = 0;
	boxIndexOffset_ = 0;
	boxIndexCnt_ = box.indices.size();

	UINT totalVertexCnt = box.vertices.size();
	UINT totalIndexCnt = boxIndexCnt_;

	std::vector<Vertex::Basic32> vertices(totalVertexCnt);

	for (UINT i = 0; i < box.vertices.size(); ++i) {
		vertices[i].pos = box.vertices[i].position;
		vertices[i].normal = box.vertices[i].normal;
		vertices[i].tex = box.vertices[i].texcoord;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * totalVertexCnt;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vertexInitData;
	vertexInitData.pSysMem = &vertices[0];
	HR(device_->CreateBuffer(&vbd, &vertexInitData, &boxVB_));


	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT)*totalIndexCnt;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA indexInitData;
	indexInitData.pSysMem = &box.indices[0];
	HR(device_->CreateBuffer(&ibd, &indexInitData, &boxIB_));

}