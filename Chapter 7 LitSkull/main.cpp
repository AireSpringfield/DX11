

#include"d3dapp.h"
#include"geometrygenerator.h"
#include"mathhelper.h"
#include"lighthelper.h"
#include"effects.h"
#include"vertex.h"
#include<string>
#include<fstream>

class LitSkullApp : public D3DApp {

public:
	LitSkullApp(HINSTANCE hInstance);
	~LitSkullApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();


	void OnMouseWheel(WPARAM wParam, LPARAM lParam);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildShapeGeometryBuffers();
	void BuildSkullGeometryBuffers();

private:

	float theta_ = 1.5f*MathHelper::Pi;
	float phi_ = 0.5f*MathHelper::Pi;
	float radius_ = 1.0f;
	POINT lastMousepos_ = { 0,0 };

	/***************************/
	ID3D11Buffer *shapesVB_ = nullptr;
	ID3D11Buffer *shapesIB_ = nullptr;

	ID3D11Buffer *skullVB_ = nullptr;
	ID3D11Buffer *skullIB_ = nullptr;

	INT boxVertexOffset_;
	INT gridVertexOffset_;
	INT sphereVertexOffset_;
	INT cylinderVertexOffset_;

	UINT boxIndexOffset_;
	UINT gridIndexOffset_;
	UINT sphereIndexOffset_;
	UINT cylinderIndexOffset_;

	UINT boxIndexCnt_;
	UINT gridIndexCnt_;
	UINT sphereIndexCnt_;
	UINT cylinderIndexCnt_;

	UINT skullIndexCnt_;


	/****************************/
	DirectionalLight dirLights_[3];
	UINT lightCnt_;
	Material gridMaterial_;
	Material boxMaterial_;
	Material cylinderMaterial_;
	Material sphereMaterial_;
	Material skullMaterial_;

	/******************************/
	XMFLOAT3 eyeposInWorld_ = XMFLOAT3(0.0f, 0.0f, 0.0f);

	XMFLOAT4X4 sphereWorld_[10];
	XMFLOAT4X4 cylinderWorld_[10];
	XMFLOAT4X4 boxWorld_;
	XMFLOAT4X4 gridWorld_;
	XMFLOAT4X4 skullWorld_;

	XMFLOAT4X4 view_ = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 proj_ = MathHelper::XMFloat4x4Identity();

	






	
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	
	LitSkullApp app(hInstance);

	if (!app.Init())
		return 0;

	return app.Run();
}

LitSkullApp::LitSkullApp(HINSTANCE hInstance)
	:D3DApp(hInstance) {
	main_wnd_caption_ = L"LitSkull Demo";

	gridWorld_ = MathHelper::XMFloat4x4Identity();

	XMMATRIX boxScale = XMMatrixScaling(3.0f, 1.0f, 3.0f);
	XMMATRIX boxTranslate = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	XMStoreFloat4x4(&boxWorld_, XMMatrixMultiply(boxScale, boxTranslate));

	XMMATRIX skullScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX skullTranslate = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&skullWorld_, XMMatrixMultiply(skullScale, skullTranslate));

	for (int i = 0; i < 5; ++i)
	{
		XMStoreFloat4x4(&cylinderWorld_[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i*5.0f));
		XMStoreFloat4x4(&cylinderWorld_[i * 2 + 1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i*5.0f));

		XMStoreFloat4x4(&sphereWorld_[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i*5.0f));
		XMStoreFloat4x4(&sphereWorld_[i * 2 + 1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i*5.0f));
	}

	dirLights_[0].ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	dirLights_[0].diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	dirLights_[0].specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	dirLights_[0].direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	dirLights_[1].ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	dirLights_[1].diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	dirLights_[1].specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	dirLights_[1].direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	dirLights_[2].ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	dirLights_[2].diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	dirLights_[2].specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	dirLights_[2].direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	gridMaterial_.ambient = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	gridMaterial_.diffuse = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	gridMaterial_.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	cylinderMaterial_.ambient = XMFLOAT4(0.7f, 0.85f, 0.7f, 1.0f);
	cylinderMaterial_.diffuse = XMFLOAT4(0.7f, 0.85f, 0.7f, 1.0f);
	cylinderMaterial_.specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);

	sphereMaterial_.ambient = XMFLOAT4(0.1f, 0.2f, 0.3f, 1.0f);
	sphereMaterial_.diffuse = XMFLOAT4(0.2f, 0.4f, 0.6f, 1.0f);
	sphereMaterial_.specular = XMFLOAT4(0.9f, 0.9f, 0.9f, 16.0f);

	boxMaterial_.ambient = XMFLOAT4(0.651f, 0.5f, 0.392f, 1.0f);
	boxMaterial_.diffuse = XMFLOAT4(0.651f, 0.5f, 0.392f, 1.0f);
	boxMaterial_.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	skullMaterial_.ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	skullMaterial_.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	skullMaterial_.specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
}

LitSkullApp::~LitSkullApp() {
	ReleaseCOM(shapesVB_);
	ReleaseCOM(shapesIB_);
	ReleaseCOM(skullVB_);
	ReleaseCOM(skullIB_);

	Effects::DestoryAll();
	InputLayouts::DestroyAll();
}



bool LitSkullApp::Init() {
	if (!D3DApp::Init())
		// D3DApp is a base class, so we can directly call its members using "::" 
		// without an instantiated D3DApp object. 
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(device_);
	InputLayouts::InitAll(device_);

	BuildShapeGeometryBuffers();
	BuildSkullGeometryBuffers();

	return true;
}

void LitSkullApp::OnResize() {
	D3DApp::OnResize();

	XMMATRIX proj = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&proj_, proj);
}

void LitSkullApp::UpdateScene(float dt) {
	float x = radius_*sinf(phi_)*cosf(theta_);
	float z = radius_*sinf(phi_)*sinf(theta_);
	float y = radius_*cosf(phi_);

	eyeposInWorld_ = XMFLOAT3(x, y, z);

	// Create the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&view_, view);

	/****************************************************/
	// Switch the number of lights based on key presses.
	/****************************************************/
	// For GetAsyncKeyState, refer to
	// https://msdn.microsoft.com/en-us/library/windows/desktop/ms646293(v=vs.85).aspx
	// "If the most significant bit (& 0x8000) is set, the key is down, 
	// and if the least significant bit is set, the key was 
	// pressed after the previous call to GetAsyncKeyState."
	if (GetAsyncKeyState('0') & 0x8000)
		lightCnt_ = 0;

	if (GetAsyncKeyState('1') & 0x8000)
		lightCnt_ = 1;

	if (GetAsyncKeyState('2') & 0x8000)
		lightCnt_ = 2;

	if (GetAsyncKeyState('3') & 0x8000)
		lightCnt_ = 3;

}

void LitSkullApp::DrawScene() {
	immediate_context_->ClearRenderTargetView(render_target_view_, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	immediate_context_->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	immediate_context_->IASetInputLayout(InputLayouts::posNormal);
	immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT strides[1] = { sizeof(Vertex::PosNormal) };
	UINT offsets[1] = { 0 };

	XMMATRIX view = XMLoadFloat4x4(&view_);
	XMMATRIX proj = XMLoadFloat4x4(&proj_);
	XMMATRIX viewProj = view*proj;

	/***********************************/
	// Set PER FRAME constants.
	/***********************************/
	Effects::basicFX->SetDirLights(dirLights_);
	Effects::basicFX->SetEyePosW(eyeposInWorld_);

	// Pick out the tech to use.
	ID3DX11EffectTechnique *activeTech = Effects::basicFX->light1Tech;
	switch (lightCnt_) {
	case 1:
		activeTech = Effects::basicFX->light1Tech;
		break;
	case 2:
		activeTech = Effects::basicFX->light2Tech;
		break;
	case 3:
		activeTech = Effects::basicFX->light3Tech;
		break;

	}

	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p) {


		/***************************************************/
		// Apply the tech and draw different geometry objects.
		/***************************************************/


		immediate_context_->IASetVertexBuffers(0, 1, &shapesVB_, strides, offsets);
		immediate_context_->IASetIndexBuffer(shapesIB_, DXGI_FORMAT_R32_UINT, 0);

		// Draw the grid.
		XMMATRIX world = XMLoadFloat4x4(&gridWorld_);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;

		Effects::basicFX->SetWorld(world);
		Effects::basicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::basicFX->SetWorldViewProj(worldViewProj);
		Effects::basicFX->SetMaterial(gridMaterial_);

		
		activeTech->GetPassByIndex(p)->Apply(0, immediate_context_);
		immediate_context_->DrawIndexed(gridIndexCnt_, gridIndexOffset_, gridVertexOffset_);

		// Draw the box.
		world = XMLoadFloat4x4(&boxWorld_);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		Effects::basicFX->SetWorld(world);
		Effects::basicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::basicFX->SetWorldViewProj(worldViewProj);
		Effects::basicFX->SetMaterial(boxMaterial_);

		activeTech->GetPassByIndex(p)->Apply(0, immediate_context_);
		immediate_context_->DrawIndexed(boxIndexCnt_, boxIndexOffset_, boxVertexOffset_);

		// Draw the cylinders.
		for (int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&cylinderWorld_[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world*view*proj;

			Effects::basicFX->SetWorld(world);
			Effects::basicFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::basicFX->SetWorldViewProj(worldViewProj);
			Effects::basicFX->SetMaterial(cylinderMaterial_);

			activeTech->GetPassByIndex(p)->Apply(0, immediate_context_);
			immediate_context_->DrawIndexed(cylinderIndexCnt_, cylinderIndexOffset_, cylinderVertexOffset_);
		}

		// Draw the spheres.
		for (int i = 0; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&sphereWorld_[i]);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world*view*proj;

			Effects::basicFX->SetWorld(world);
			Effects::basicFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::basicFX->SetWorldViewProj(worldViewProj);
			Effects::basicFX->SetMaterial(sphereMaterial_);

			activeTech->GetPassByIndex(p)->Apply(0, immediate_context_);
			immediate_context_->DrawIndexed(sphereIndexCnt_, sphereIndexOffset_, sphereVertexOffset_);
		}

		// Draw the skull.

		immediate_context_->IASetVertexBuffers(0, 1, &skullVB_, strides, offsets);
		immediate_context_->IASetIndexBuffer(skullIB_, DXGI_FORMAT_R32_UINT, 0);

		world = XMLoadFloat4x4(&skullWorld_);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		Effects::basicFX->SetWorld(world);
		Effects::basicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::basicFX->SetWorldViewProj(worldViewProj);
		Effects::basicFX->SetMaterial(skullMaterial_);

		activeTech->GetPassByIndex(p)->Apply(0, immediate_context_);
		immediate_context_->DrawIndexed(skullIndexCnt_, 0, 0);
	}

	HR(swap_chain_->Present(0, 0));
	


}

void LitSkullApp::OnMouseWheel(WPARAM wParam, LPARAM lParam) {
	short delta = GET_WHEEL_DELTA_WPARAM(wParam);
	int inc = -delta / WHEEL_DELTA;
	radius_ += inc*5.0f;
	radius_ = MathHelper::Clamp(radius_, 0.5f, 100.0f);
}

void LitSkullApp::OnMouseUp(WPARAM btnState, int x, int y) {
	ReleaseCapture();
}
void LitSkullApp::OnMouseDown(WPARAM btnState, int x, int y) {
	lastMousepos_.x = x;
	lastMousepos_.y = y;
	SetCapture(main_wnd_);
}
void LitSkullApp::OnMouseMove(WPARAM btnState, int x, int y) {
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - lastMousepos_.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - lastMousepos_.y));

		// Update angles based on input to orbit camera around box.
		theta_ += dx;
		phi_ += dy;

		// Restrict the angle mPhi.
		phi_ = MathHelper::Clamp(phi_, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.01 unit in the scene.
		float dx = 0.01f*static_cast<float>(x - lastMousepos_.x);
		float dy = 0.01f*static_cast<float>(y - lastMousepos_.y);

		// Update the camera radius based on input.
		radius_ += dx - dy;

		// Restrict the radius.
		radius_ = MathHelper::Clamp(radius_, 3.0f, 200.0f);
	}



	lastMousepos_.x = x;
	lastMousepos_.y = y;
}


void LitSkullApp::BuildShapeGeometryBuffers()
{
	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData grid;
	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData cylinder;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
	geoGen.CreateSphere(0.5f, 20, 20, sphere);
	geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	boxVertexOffset_ = 0;
	gridVertexOffset_ = box.vertices.size();
	sphereVertexOffset_ = gridVertexOffset_ + grid.vertices.size();
	cylinderVertexOffset_ = sphereVertexOffset_ + sphere.vertices.size();

	// Cache the index count of each object.
	boxIndexCnt_ = box.indices.size();
	gridIndexCnt_ = grid.indices.size();
	sphereIndexCnt_ = sphere.indices.size();
	cylinderIndexCnt_ = cylinder.indices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	boxIndexOffset_ = 0;
	gridIndexOffset_ = boxIndexCnt_;
	sphereIndexOffset_ = gridIndexOffset_+ gridIndexCnt_;
	cylinderIndexOffset_ = sphereIndexOffset_ + sphereIndexCnt_;

	UINT totalVertexCount =
		box.vertices.size() +
		grid.vertices.size() +
		sphere.vertices.size() +
		cylinder.vertices.size();

	UINT totalIndexCount =
		boxIndexCnt_ +
		gridIndexCnt_ +
		sphereIndexCnt_ +
		cylinderIndexCnt_;

	//
	// Extractnthe vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	std::vector<Vertex::PosNormal> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.vertices.size(); ++i, ++k)
	{
		vertices[k].pos = box.vertices[i].position;
		vertices[k].normal = box.vertices[i].normal;
	}

	for (size_t i = 0; i < grid.vertices.size(); ++i, ++k)
	{
		vertices[k].pos = grid.vertices[i].position;
		vertices[k].normal = grid.vertices[i].normal;
	}

	for (size_t i = 0; i < sphere.vertices.size(); ++i, ++k)
	{
		vertices[k].pos = sphere.vertices[i].position;
		vertices[k].normal = sphere.vertices[i].normal;
	}

	for (size_t i = 0; i < cylinder.vertices.size(); ++i, ++k)
	{
		vertices[k].pos = cylinder.vertices[i].position;
		vertices[k].normal = cylinder.vertices[i].normal;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::PosNormal) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(device_->CreateBuffer(&vbd, &vinitData, &shapesVB_));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.indices.begin(), box.indices.end());
	indices.insert(indices.end(), grid.indices.begin(), grid.indices.end());
	indices.insert(indices.end(), sphere.indices.begin(), sphere.indices.end());
	indices.insert(indices.end(), cylinder.indices.begin(), cylinder.indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(device_->CreateBuffer(&ibd, &iinitData, &shapesIB_));
}

void LitSkullApp::BuildSkullGeometryBuffers()
{
	std::ifstream fin("Models/skull.txt");

	if (!fin)
	{
		MessageBox(0, L"Models/skull.txt not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	std::vector<Vertex::PosNormal> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].pos.x >> vertices[i].pos.y >> vertices[i].pos.z;
		fin >> vertices[i].normal.x >> vertices[i].normal.y >> vertices[i].normal.z;
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	skullIndexCnt_ = 3 * tcount;
	std::vector<UINT> indices(skullIndexCnt_);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}

	fin.close();

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::PosNormal) * vcount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(device_->CreateBuffer(&vbd, &vinitData, &skullVB_));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * skullIndexCnt_;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(device_->CreateBuffer(&ibd, &iinitData, &skullIB_));
}

