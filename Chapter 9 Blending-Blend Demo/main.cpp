//***************************************************************************************
// BlendDemo.cpp by Frank Luna (C) 2011 All Rights Reserved.
//
// Demonstrates blending, HLSL clip(), and fogging.
//
// Controls:
//		Hold the left mouse button down and move the mouse to rotate.
//      Hold the right mouse button down to zoom in and out.
//
//      Press '1' - Lighting only render mode.
//      Press '2' - Texture render mode.
//      Press '3' - Fog render mode.
//
//***************************************************************************************

/**********************************************************************/
// Exercise 1: Change sample mask to 0xfffffffe before drawing waves.
/**********************************************************************/
// Exercise 3: Draw the waves before hills.
/**********************************************************************/
// Exercise 6: Create a blend render state so that the overlay only affect
// red and green channel.
/**********************************************************************/


#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "geometrygenerator.h"
#include "mathhelper.h"
#include "lighthelper.h"
#include "effects.h"
#include "vertex.h"
#include "renderstates.h"
#include "waves.h"

using namespace DirectX;

enum RenderOptions
{
	Lighting = 0,
	Textures = 1 ,
	TexturesAndFog = 2 
};

class BlendApp : public D3DApp
{
public:
	BlendApp(HINSTANCE hInstance);
	~BlendApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene(); 

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	float GetHillHeight(float x, float z)const;
	XMFLOAT3 GetHillNormal(float x, float z)const;
	void BuildLandGeometryBuffers();
	void BuildWaveGeometryBuffers();
	void BuildCrateGeometryBuffers();

private:
	ID3D11Buffer* landVB_ = nullptr;
	ID3D11Buffer* landIB_ = nullptr;

	ID3D11Buffer* wavesVB_ = nullptr;
	ID3D11Buffer* wavesIB_ = nullptr;

	ID3D11Buffer* boxVB_ = nullptr;
	ID3D11Buffer* boxIB_ = nullptr;

	ID3D11ShaderResourceView* grassMapSRV_ = nullptr;
	ID3D11ShaderResourceView* wavesMapSRV_ = nullptr;
	ID3D11ShaderResourceView* boxMapSRV_ = nullptr;

	Waves waves_;

	DirectionalLight dirLights_[3];
	Material landMaterial_;
	Material wavesMaterial_;
	Material boxMaterial_;

	XMFLOAT4X4 grassTexTransform_ = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 waterTexTransform_ = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 landWorld_ = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 wavesWorld_ = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 boxWorld_ = MathHelper::XMFloat4x4Identity();

	XMFLOAT4X4 view_ = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 proj_ = MathHelper::XMFloat4x4Identity();

	UINT landIndexCnt_ = 0;

	XMFLOAT2 waterTexOffset_ = XMFLOAT2(0.0f, 0.0f);

	RenderOptions renderOptions_ = TexturesAndFog;

	XMFLOAT3 eyePosW_ = XMFLOAT3(0.0f, 0.0f, 0.0f);

	float theta_ = 1.5f*MathHelper::Pi;
	float phi_ = 0.5f*MathHelper::Pi;
	float radius_ = 50.0f;

	POINT lastMousePos_ = { 0,0 };
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	BlendApp theApp(hInstance);
	
	if( !theApp.Init() )
		return 0;
	
	return theApp.Run();
}

BlendApp::BlendApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
	main_wnd_caption_ = L"Blend Demo";
	enable_msaa4x_ = false;



	XMMATRIX boxScale = XMMatrixScaling(15.0f, 15.0f, 15.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(8.0f, 5.0f, -15.0f);
	XMStoreFloat4x4(&boxWorld_, boxScale*boxOffset);

	XMMATRIX grassTexScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);
	XMStoreFloat4x4(&grassTexTransform_, grassTexScale);

	dirLights_[0].ambient  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	dirLights_[0].diffuse  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	dirLights_[0].specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	dirLights_[0].direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	dirLights_[1].ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	dirLights_[1].diffuse  = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	dirLights_[1].specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	dirLights_[1].direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	dirLights_[2].ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	dirLights_[2].diffuse  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	dirLights_[2].specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	dirLights_[2].direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	landMaterial_.ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	landMaterial_.diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	landMaterial_.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	wavesMaterial_.ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	wavesMaterial_.diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);  // Note here, 0.5f for tarnsparency.
	wavesMaterial_.specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);

	boxMaterial_.ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	boxMaterial_.diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	boxMaterial_.specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
}

BlendApp::~BlendApp()
{
	immediate_context_->ClearState();
	ReleaseCOM(landVB_);
	ReleaseCOM(landIB_);
	ReleaseCOM(wavesVB_);
	ReleaseCOM(wavesIB_);
	ReleaseCOM(boxVB_);
	ReleaseCOM(boxIB_);
	ReleaseCOM(grassMapSRV_);
	ReleaseCOM(wavesMapSRV_);
	ReleaseCOM(boxMapSRV_);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool BlendApp::Init()
{
	if(!D3DApp::Init())
		return false;

	waves_.Init(160, 160, 1.0f, 0.03f, 5.0f, 0.3f);

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(device_);
	InputLayouts::InitAll(device_);
	RenderStates::InitAll(device_);

	
	CreateDDSShaderResourceViewFromFile(device_, L"Textures/grass.dds", &grassMapSRV_);
	CreateDDSShaderResourceViewFromFile(device_, L"Textures/water2.dds", &wavesMapSRV_);
	CreateDDSShaderResourceViewFromFile(device_, L"Textures/WireFence.dds", &boxMapSRV_);

	BuildLandGeometryBuffers();
	BuildWaveGeometryBuffers();
	BuildCrateGeometryBuffers();

	return true;
}

void BlendApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&proj_, P);
}

void BlendApp::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = radius_*sinf(phi_)*cosf(theta_);
	float z = radius_*sinf(phi_)*sinf(theta_);
	float y = radius_*cosf(phi_);

	eyePosW_ = XMFLOAT3(x, y, z);

	// Build the view matrix.
	XMVECTOR pos    = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&view_, V);

	//
	// Every quarter second, generate a random wave.
	//
	static float t_base = 0.0f;
	if( (timer_.TotalTime() - t_base) >= 0.1f )
	{
		t_base += 0.1f;
 
		DWORD i = 5 + rand() % (waves_.RowCount()-10);
		DWORD j = 5 + rand() % (waves_.ColumnCount()-10);

		float r = MathHelper::RandF(0.5f, 1.0f);

		waves_.Disturb(i, j, r);
	}

	waves_.Update(dt);

	//
	// Update the wave vertex buffer with the new solution.
	//
	
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(immediate_context_->Map(wavesVB_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

	Vertex::Basic32* v = reinterpret_cast<Vertex::Basic32*>(mappedData.pData);
	for(UINT i = 0; i < waves_.VertexCount(); ++i)
	{
		v[i].Pos    = waves_[i];
		v[i].Normal = waves_.Normal(i);

		// Derive tex-coords in [0,1] from position.
		v[i].Tex.x  = 0.5f + waves_[i].x / waves_.Width();
		v[i].Tex.y  = 0.5f - waves_[i].z / waves_.Depth();
	}

	immediate_context_->Unmap(wavesVB_, 0);

	//
	// Animate water texture coordinates.
	//

	// Tile water texture.
	XMMATRIX wavesScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);

	// Translate texture over time.
	waterTexOffset_.y += 0.05f*dt;
	waterTexOffset_.x += 0.1f*dt;	
	XMMATRIX wavesOffset = XMMatrixTranslation(waterTexOffset_.x, waterTexOffset_.y, 0.0f);

	// Combine scale and translation.
	XMStoreFloat4x4(&waterTexTransform_, wavesScale*wavesOffset);

	//
	// Switch the render mode based in key input.
	//
	if( GetAsyncKeyState('1') & 0x8000 )
		renderOptions_ = RenderOptions::Lighting; 

	if( GetAsyncKeyState('2') & 0x8000 )
		renderOptions_ = RenderOptions::Textures; 

	if( GetAsyncKeyState('3') & 0x8000 )
		renderOptions_ = RenderOptions::TexturesAndFog; 
}

void BlendApp::DrawScene()
{
	immediate_context_->ClearRenderTargetView(render_target_view_, reinterpret_cast<const float*>(&Colors::Silver));
	immediate_context_->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	immediate_context_->IASetInputLayout(InputLayouts::Basic32);
    immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
 
	// Explicit blend factors are not used in this demo.
	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};

	UINT stride = sizeof(Vertex::Basic32);
    UINT offset = 0;
 
	XMMATRIX view  = XMLoadFloat4x4(&view_);
	XMMATRIX proj  = XMLoadFloat4x4(&proj_);
	XMMATRIX viewProj = view*proj;

	// Set per frame constants.
	Effects::BasicFX->SetDirLights(dirLights_);
	Effects::BasicFX->SetEyePosW(eyePosW_);
	Effects::BasicFX->SetFogColor(Colors::Silver);
	Effects::BasicFX->SetFogStart(15.0f);
	Effects::BasicFX->SetFogRange(175.0f);
 
	ID3DX11EffectTechnique* boxTech;
	ID3DX11EffectTechnique* landAndWavesTech;

	switch(renderOptions_)
	{
	case RenderOptions::Lighting:
		boxTech = Effects::BasicFX->Light3Tech;
		landAndWavesTech = Effects::BasicFX->Light3Tech;
		break;
	case RenderOptions::Textures:
		boxTech = Effects::BasicFX->Light3TexAlphaClipTech;
		landAndWavesTech = Effects::BasicFX->Light3TexTech;
		break;
	case RenderOptions::TexturesAndFog:
		boxTech = Effects::BasicFX->Light3TexAlphaClipFogTech;
		landAndWavesTech = Effects::BasicFX->Light3TexFogTech;
		break;
	}

	D3DX11_TECHNIQUE_DESC techDesc;

	//
	// Draw the box with alpha clipping.
	// 

	boxTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		immediate_context_->IASetVertexBuffers(0, 1, &boxVB_, &stride, &offset);
		immediate_context_->IASetIndexBuffer(boxIB_, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&boxWorld_);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;
		
		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(boxMaterial_);
		Effects::BasicFX->SetDiffuseMap(boxMapSRV_);

		immediate_context_->RSSetState(RenderStates::NoCullRS);
		boxTech->GetPassByIndex(p)->Apply(0, immediate_context_);
		immediate_context_->DrawIndexed(36, 0, 0);

		// Restore default render state.
		immediate_context_->RSSetState(0);
	}

	//
	// Draw the hills and water with texture and fog (no alpha clipping needed).
	//

	landAndWavesTech->GetDesc( &techDesc );
    for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		
		//
		// Draw the hills.
		//
		immediate_context_->IASetVertexBuffers(0, 1, &landVB_, &stride, &offset);
		immediate_context_->IASetIndexBuffer(landIB_, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&landWorld_);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;
		
		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&grassTexTransform_));
		Effects::BasicFX->SetMaterial(landMaterial_);
		Effects::BasicFX->SetDiffuseMap(grassMapSRV_);

		landAndWavesTech->GetPassByIndex(p)->Apply(0, immediate_context_);
		immediate_context_->DrawIndexed(landIndexCnt_, 0, 0);

		//
		// Draw the waves.
		//
		
	

		immediate_context_->IASetVertexBuffers(0, 1, &wavesVB_, &stride, &offset);
		immediate_context_->IASetIndexBuffer(wavesIB_, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		world = XMLoadFloat4x4(&wavesWorld_);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;
		
		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&waterTexTransform_));
		Effects::BasicFX->SetMaterial(wavesMaterial_);
		Effects::BasicFX->SetDiffuseMap(wavesMapSRV_);

		immediate_context_->OMSetBlendState(RenderStates::TransparentBS, blendFactor, 0xffffffff);

		// For exercise 1.
		// immediate_context_->OMSetBlendState(RenderStates::TransparentBS, blendFactor, 0xfffffffe);

		// For exercise 6: Only write to red and green channel.
		immediate_context_->OMSetBlendState(RenderStates::TransparentRedGreenOnlyBS, blendFactor, 0xfffffffff);

		landAndWavesTech->GetPassByIndex(p)->Apply(0, immediate_context_);
		immediate_context_->DrawIndexed(3*waves_.TriangleCount(), 0, 0);

		// Restore default blend state
		immediate_context_->OMSetBlendState(0, blendFactor, 0xffffffff);
		



		/***********************************************************/
		// Exercise 3: draw the waters first.
		// When drawing hills, blending state is reset to default so
		// whether a pixel of hills is visible is simply determined 
		// by depth.
		/***********************************************************/

		/*
		//
		// Draw the waves.
		//



		immediate_context_->IASetVertexBuffers(0, 1, &wavesVB_, &stride, &offset);
		immediate_context_->IASetIndexBuffer(wavesIB_, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&wavesWorld_);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&waterTexTransform_));
		Effects::BasicFX->SetMaterial(wavesMaterial_);
		Effects::BasicFX->SetDiffuseMap(wavesMapSRV_);

		immediate_context_->OMSetBlendState(RenderStates::TransparentBS, blendFactor, 0xffffffff);

		landAndWavesTech->GetPassByIndex(p)->Apply(0, immediate_context_);
		immediate_context_->DrawIndexed(3 * waves_.TriangleCount(), 0, 0);
		

		//
		// Draw the hills.
		//
		
		// Restore default blend state
		immediate_context_->OMSetBlendState(0, blendFactor, 0xffffffff);


		immediate_context_->IASetVertexBuffers(0, 1, &landVB_, &stride, &offset);
		immediate_context_->IASetIndexBuffer(landIB_, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		world = XMLoadFloat4x4(&landWorld_);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&grassTexTransform_));
		Effects::BasicFX->SetMaterial(landMaterial_);
		Effects::BasicFX->SetDiffuseMap(grassMapSRV_);

		landAndWavesTech->GetPassByIndex(p)->Apply(0, immediate_context_);
		immediate_context_->DrawIndexed(landIndexCnt_, 0, 0);
		*/


    }

	HR(swap_chain_->Present(0, 0));
}

void BlendApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	lastMousePos_.x = x;
	lastMousePos_.y = y;

	SetCapture(main_wnd_);
}

void BlendApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void BlendApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if( (btnState & MK_LBUTTON) != 0 )
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - lastMousePos_.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - lastMousePos_.y));

		// Update angles based on input to orbit camera around box.
		theta_ += dx;
		phi_   += dy;

		// Restrict the angle phi_.
		phi_ = MathHelper::Clamp(phi_, 0.1f, MathHelper::Pi-0.1f);
	}
	else if( (btnState & MK_RBUTTON) != 0 )
	{
		// Make each pixel correspond to 0.01 unit in the scene.
		float dx = 0.1f*static_cast<float>(x - lastMousePos_.x);
		float dy = 0.1f*static_cast<float>(y - lastMousePos_.y);

		// Update the camera radius based on input.
		radius_ += dx - dy;

		// Restrict the radius.
		radius_ = MathHelper::Clamp(radius_, 20.0f, 500.0f);
	}

	lastMousePos_.x = x;
	lastMousePos_.y = y;
}

float BlendApp::GetHillHeight(float x, float z)const
{
	return 0.3f*( z*sinf(0.1f*x) + x*cosf(0.1f*z) );
}

XMFLOAT3 BlendApp::GetHillNormal(float x, float z)const
{
	// n = (-df/dx, 1, -df/dz)
	XMFLOAT3 n(
		-0.03f*z*cosf(0.1f*x) - 0.3f*cosf(0.1f*z),
		1.0f,
		-0.3f*sinf(0.1f*x) + 0.03f*x*sinf(0.1f*z));
	
	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}

void BlendApp::BuildLandGeometryBuffers()
{
	GeometryGenerator::MeshData grid;
 
	GeometryGenerator geoGen;

	geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

	landIndexCnt_ = grid.indices.size();

	//
	// Extract the vertex elements we are interested and apply the height function to
	// each vertex.  
	//

	std::vector<Vertex::Basic32> vertices(grid.vertices.size());
	for(UINT i = 0; i < grid.vertices.size(); ++i)
	{
		XMFLOAT3 p = grid.vertices[i].position;

		p.y = GetHillHeight(p.x, p.z);
		
		vertices[i].Pos    = p;
		vertices[i].Normal = GetHillNormal(p.x, p.z);
		vertices[i].Tex    = grid.vertices[i].texcoord;
	}

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * grid.vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(device_->CreateBuffer(&vbd, &vinitData, &landVB_));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * landIndexCnt_;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &grid.indices[0];
    HR(device_->CreateBuffer(&ibd, &iinitData, &landIB_));
}

void BlendApp::BuildWaveGeometryBuffers()
{
	// Create the vertex buffer.  Note that we allocate space only, as
	// we will be updating the data every time step of the simulation.

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * waves_.VertexCount();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vbd.MiscFlags = 0;
    HR(device_->CreateBuffer(&vbd, 0, &wavesVB_));


	// Create the index buffer.  The index buffer is fixed, so we only 
	// need to create and set once.

	std::vector<UINT> indices(3*waves_.TriangleCount()); // 3 indices per face

	// Iterate over each quad.
	UINT m = waves_.RowCount();
	UINT n = waves_.ColumnCount();
	int k = 0;
	for(UINT i = 0; i < m-1; ++i)
	{
		for(DWORD j = 0; j < n-1; ++j)
		{
			indices[k]   = i*n+j;
			indices[k+1] = i*n+j+1;
			indices[k+2] = (i+1)*n+j;

			indices[k+3] = (i+1)*n+j;
			indices[k+4] = i*n+j+1;
			indices[k+5] = (i+1)*n+j+1;

			k += 6; // next quad
		}
	}

	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * indices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    HR(device_->CreateBuffer(&ibd, &iinitData, &wavesIB_));
}

void BlendApp::BuildCrateGeometryBuffers()
{
	GeometryGenerator::MeshData box;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	std::vector<Vertex::Basic32> vertices(box.vertices.size());

	for(UINT i = 0; i < box.vertices.size(); ++i)
	{
		vertices[i].Pos    = box.vertices[i].position;
		vertices[i].Normal = box.vertices[i].normal;
		vertices[i].Tex    = box.vertices[i].texcoord;
	}

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * box.vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(device_->CreateBuffer(&vbd, &vinitData, &boxVB_));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * box.indices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &box.indices[0];
    HR(device_->CreateBuffer(&ibd, &iinitData, &boxIB_));
}