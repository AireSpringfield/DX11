//***************************************************************************************
// TreeBillboardDemo.cpp by Frank Luna (C) 2011 All Rights Reserved.
//
// Demonstrates the geometry shader, texture arrays, and alpha to coverage.
//
// Controls:
//		Hold the left mouse button down and move the mouse to rotate.
//      Hold the right mouse button down to zoom in and out.
//
//      Press '1' - Lighting only render mode.
//      Press '2' - Texture render mode.
//      Press '3' - Fog render mode.
//      Press 'r' - Alpha-to-coverage on.
//      Press 't' - Alpha-to-coverage off.
//
//***************************************************************************************

#include "d3dapp.h"
#include "d3dx11Effect.h"
#include "geometrygenerator.h"
#include "mathhelper.h"
#include "lighthelper.h"
#include "Effects.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "waves.h"

enum RenderOptions
{
	Lighting = 0,
	Textures = 1,
	TexturesAndFog = 2
};

class TreeBillboardApp : public D3DApp
{
public:
	TreeBillboardApp(HINSTANCE hInstance);
	~TreeBillboardApp();

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
	void BuildTreeSpritesBuffer();
	void DrawTreeSprites(CXMMATRIX viewProj);

private:
	ID3D11Buffer* mLandVB;
	ID3D11Buffer* mLandIB;

	ID3D11Buffer* mWavesVB;
	ID3D11Buffer* mWavesIB;

	ID3D11Buffer* mBoxVB;
	ID3D11Buffer* mBoxIB;

	ID3D11Buffer* mTreeSpritesVB;

	ID3D11ShaderResourceView* mGrassMapSRV;
	ID3D11ShaderResourceView* mWavesMapSRV;
	ID3D11ShaderResourceView* mBoxMapSRV;
	ID3D11ShaderResourceView* mTreeTextureMapArraySRV;

	Waves mWaves;

	DirectionalLight mDirLights[3];
	Material mLandMat;
	Material mWavesMat;
	Material mBoxMat;
	Material mTreeMat;

	XMFLOAT4X4 mGrassTexTransform = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 mWaterTexTransform = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 mLandWorld = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 mWavesWorld = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 mBoxWorld = MathHelper::XMFloat4x4Identity();

	XMFLOAT4X4 mView = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 mProj = MathHelper::XMFloat4x4Identity();

	UINT mLandIndexCount;

	static const UINT TreeCount = 16;

	bool mAlphaToCoverageOn;

	XMFLOAT2 mWaterTexOffset;

	RenderOptions mRenderOptions;

	XMFLOAT3 mEyePosW;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos = { 0,0 };
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	TreeBillboardApp theApp(hInstance);
	
	if( !theApp.Init() )
		return 0;
	
	return theApp.Run();
}

TreeBillboardApp::TreeBillboardApp(HINSTANCE hInstance)
: D3DApp(hInstance), mLandVB(0), mLandIB(0), mWavesVB(0), mWavesIB(0), mBoxVB(0), mBoxIB(0), mTreeSpritesVB(0),
  mGrassMapSRV(0), mWavesMapSRV(0), mBoxMapSRV(0), mAlphaToCoverageOn(true),
  mWaterTexOffset(0.0f, 0.0f), mEyePosW(0.0f, 0.0f, 0.0f), mLandIndexCount(0), mRenderOptions(RenderOptions::TexturesAndFog),
  mTheta(1.3f*MathHelper::Pi), mPhi(0.4f*MathHelper::Pi), mRadius(80.0f)
{
	main_wnd_caption_ = L"Tree Billboard Demo";
	enable_msaa4x_ = true;


	

	XMMATRIX boxScale = XMMatrixScaling(15.0f, 15.0f, 15.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(8.0f, 5.0f, -15.0f);
	XMStoreFloat4x4(&mBoxWorld, boxScale*boxOffset);

	XMMATRIX grassTexScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);
	XMStoreFloat4x4(&mGrassTexTransform, grassTexScale);

	mDirLights[0].ambient  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].diffuse  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	mDirLights[1].ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].diffuse  = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	mDirLights[1].specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	mDirLights[1].direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mDirLights[2].ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].diffuse  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	mLandMat.ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mLandMat.diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mLandMat.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	mWavesMat.ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mWavesMat.diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	mWavesMat.specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);

	mBoxMat.ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mBoxMat.diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	mTreeMat.ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mTreeMat.diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mTreeMat.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
}

TreeBillboardApp::~TreeBillboardApp()
{
	immediate_context_->ClearState();
	ReleaseCOM(mLandVB);
	ReleaseCOM(mLandIB);
	ReleaseCOM(mWavesVB);
	ReleaseCOM(mWavesIB);
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	ReleaseCOM(mTreeSpritesVB);
	ReleaseCOM(mGrassMapSRV);
	ReleaseCOM(mWavesMapSRV);
	ReleaseCOM(mBoxMapSRV);
	ReleaseCOM(mTreeTextureMapArraySRV);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool TreeBillboardApp::Init()
{
	if(!D3DApp::Init())
		return false;

	mWaves.Init(160, 160, 1.0f, 0.03f, 5.0f, 0.3f);

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(device_);
	InputLayouts::InitAll(device_);
	RenderStates::InitAll(device_);

	CreateDDSShaderResourceViewFromFile(device_, L"Textures/grass.dds", &mGrassMapSRV);

	CreateDDSShaderResourceViewFromFile(device_, L"Textures/water2.dds",&mWavesMapSRV);

	CreateDDSShaderResourceViewFromFile(device_, L"Textures/WireFence.dds",&mBoxMapSRV);

	std::vector<std::wstring> treeFilenames;
	treeFilenames.push_back(L"Textures/tree0.dds");
	treeFilenames.push_back(L"Textures/tree1.dds");
	treeFilenames.push_back(L"Textures/tree2.dds");
	treeFilenames.push_back(L"Textures/tree3.dds");
	

	mTreeTextureMapArraySRV = d3dHelper::CreateTexture2DArraySRV(
		device_, immediate_context_, treeFilenames);

	BuildLandGeometryBuffers();
	BuildWaveGeometryBuffers();
	BuildCrateGeometryBuffers();
	BuildTreeSpritesBuffer();

	return true;
}

void TreeBillboardApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void TreeBillboardApp::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius*sinf(mPhi)*cosf(mTheta);
	float z = mRadius*sinf(mPhi)*sinf(mTheta);
	float y = mRadius*cosf(mPhi);

	mEyePosW = XMFLOAT3(x, y, z);

	// Build the view matrix.
	XMVECTOR pos    = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);

	//
	// Every quarter second, generate a random wave.
	//
	static float t_base = 0.0f;
	if( (timer_.TotalTime() - t_base) >= 0.1f )
	{
		t_base += 0.1f;
 
		DWORD i = 5 + rand() % (mWaves.RowCount()-10);
		DWORD j = 5 + rand() % (mWaves.ColumnCount()-10);

		float r = MathHelper::RandF(0.5f, 1.0f);

		mWaves.Disturb(i, j, r);
	}

	mWaves.Update(dt);

	//
	// Update the wave vertex buffer with the new solution.
	//
	
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(immediate_context_->Map(mWavesVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

	Vertex::Basic32* v = reinterpret_cast<Vertex::Basic32*>(mappedData.pData);
	for(UINT i = 0; i < mWaves.VertexCount(); ++i)
	{
		v[i].Pos    = mWaves[i];
		v[i].Normal = mWaves.Normal(i);

		// Derive tex-coords in [0,1] from position.
		v[i].Tex.x  = 0.5f + mWaves[i].x / mWaves.Width();
		v[i].Tex.y  = 0.5f - mWaves[i].z / mWaves.Depth();
	}

	immediate_context_->Unmap(mWavesVB, 0);

	//
	// Animate water texture coordinates.
	//

	// Tile water texture.
	XMMATRIX wavesScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);

	// Translate texture over time.
	mWaterTexOffset.y += 0.05f*dt;
	mWaterTexOffset.x += 0.1f*dt;	
	XMMATRIX wavesOffset = XMMatrixTranslation(mWaterTexOffset.x, mWaterTexOffset.y, 0.0f);

	// Combine scale and translation.
	XMStoreFloat4x4(&mWaterTexTransform, wavesScale*wavesOffset);

	//
	// Switch the render mode based in key input.
	//
	if( GetAsyncKeyState('1') & 0x8000 )
		mRenderOptions = RenderOptions::Lighting; 

	if( GetAsyncKeyState('2') & 0x8000 )
		mRenderOptions = RenderOptions::Textures; 

	if( GetAsyncKeyState('3') & 0x8000 )
		mRenderOptions = RenderOptions::TexturesAndFog; 

	if( GetAsyncKeyState('R') & 0x8000 )
		mAlphaToCoverageOn = true;

	if( GetAsyncKeyState('T') & 0x8000 )
		mAlphaToCoverageOn = false;
}

void TreeBillboardApp::DrawScene()
{
	immediate_context_->ClearRenderTargetView(render_target_view_, reinterpret_cast<const float*>(&Colors::Silver));
	immediate_context_->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
 
	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};
 
	XMMATRIX view  = XMLoadFloat4x4(&mView);
	XMMATRIX proj  = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view*proj;

	//
	// Draw the tree sprites
	//

	DrawTreeSprites(viewProj);

	//
	// DrawTreeSprites() changes InputLayout and PrimitiveTopology, so change it based on 
	// the geometry we draw next.
	//

	/****************************************************************/
	// When using alpha-to-coverage, make sure that alpha clip is disabled.
	// Or if a pixel is discarded at pixel shader stage, A2C tech at OM stage will not help.
	/****************************************************************/
	immediate_context_->IASetInputLayout(InputLayouts::Basic32);
    immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT stride = sizeof(Vertex::Basic32);
    UINT offset = 0;

	//
	// Set per frame constants for the rest of the objects.
	//
	Effects::BasicFX->SetDirLights(mDirLights);
	Effects::BasicFX->SetEyePosW(mEyePosW);
	Effects::BasicFX->SetFogColor(Colors::Silver);
	Effects::BasicFX->SetFogStart(15.0f);
	Effects::BasicFX->SetFogRange(175.0f);

	//
	// Figure out which technique to use.
	//
	ID3DX11EffectTechnique* boxTech;
	ID3DX11EffectTechnique* landAndWavesTech;
 
	switch(mRenderOptions)
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
	// Draw the box.
	// 

	boxTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		immediate_context_->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
		immediate_context_->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&mBoxWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;
		
		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(mBoxMat);
		Effects::BasicFX->SetDiffuseMap(mBoxMapSRV);

		//immediate_context_->OMSetBlendState(RenderStates::AlphaToCoverageBS, blendFactor, 0xffffffff);
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
		immediate_context_->IASetVertexBuffers(0, 1, &mLandVB, &stride, &offset);
		immediate_context_->IASetIndexBuffer(mLandIB, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&mLandWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;
		
		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mGrassTexTransform));
		Effects::BasicFX->SetMaterial(mLandMat);
		Effects::BasicFX->SetDiffuseMap(mGrassMapSRV);

		landAndWavesTech->GetPassByIndex(p)->Apply(0, immediate_context_);
		immediate_context_->DrawIndexed(mLandIndexCount, 0, 0);

		//
		// Draw the waves.
		//
		immediate_context_->IASetVertexBuffers(0, 1, &mWavesVB, &stride, &offset);
		immediate_context_->IASetIndexBuffer(mWavesIB, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		world = XMLoadFloat4x4(&mWavesWorld);
		worldInvTranspose = MathHelper::InverseTranspose(world);
		worldViewProj = world*view*proj;
		
		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mWaterTexTransform));
		Effects::BasicFX->SetMaterial(mWavesMat);
		Effects::BasicFX->SetDiffuseMap(mWavesMapSRV);

		immediate_context_->OMSetBlendState(RenderStates::TransparentBS, blendFactor, 0xffffffff);
		landAndWavesTech->GetPassByIndex(p)->Apply(0, immediate_context_);
		immediate_context_->DrawIndexed(3*mWaves.TriangleCount(), 0, 0);

		// Restore default blend state
		immediate_context_->OMSetBlendState(0, blendFactor, 0xffffffff);
    }

	HR(swap_chain_->Present(0, 0));
}

void TreeBillboardApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(main_wnd_);
}

void TreeBillboardApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void TreeBillboardApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if( (btnState & MK_LBUTTON) != 0 )
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi   += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi-0.1f);
	}
	else if( (btnState & MK_RBUTTON) != 0 )
	{
		// Make each pixel correspond to 0.01 unit in the scene.
		float dx = 0.1f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.1f*static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 20.0f, 500.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

float TreeBillboardApp::GetHillHeight(float x, float z)const
{
	return 0.3f*( z*sinf(0.1f*x) + x*cosf(0.1f*z) );
}

XMFLOAT3 TreeBillboardApp::GetHillNormal(float x, float z)const
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

void TreeBillboardApp::BuildLandGeometryBuffers()
{
	GeometryGenerator::MeshData grid;
 
	GeometryGenerator geoGen;

	geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

	mLandIndexCount = grid.indices.size();

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
    HR(device_->CreateBuffer(&vbd, &vinitData, &mLandVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mLandIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &grid.indices[0];
    HR(device_->CreateBuffer(&ibd, &iinitData, &mLandIB));
}

void TreeBillboardApp::BuildWaveGeometryBuffers()
{
	// Create the vertex buffer.  Note that we allocate space only, as
	// we will be updating the data every time step of the simulation.

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * mWaves.VertexCount();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vbd.MiscFlags = 0;
    HR(device_->CreateBuffer(&vbd, 0, &mWavesVB));


	// Create the index buffer.  The index buffer is fixed, so we only 
	// need to create and set once.

	std::vector<UINT> indices(3*mWaves.TriangleCount()); // 3 indices per face

	// Iterate over each quad.
	UINT m = mWaves.RowCount();
	UINT n = mWaves.ColumnCount();
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
    HR(device_->CreateBuffer(&ibd, &iinitData, &mWavesIB));
}

void TreeBillboardApp::BuildCrateGeometryBuffers()
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
    HR(device_->CreateBuffer(&vbd, &vinitData, &mBoxVB));

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
    HR(device_->CreateBuffer(&ibd, &iinitData, &mBoxIB));
}

void TreeBillboardApp::BuildTreeSpritesBuffer()
{
	Vertex::TreePointSprite v[TreeCount];

	for(UINT i = 0; i < TreeCount; ++i)
	{
		float x = MathHelper::RandF(-35.0f, 35.0f);
		float z = MathHelper::RandF(-35.0f, 35.0f);
		float y = GetHillHeight(x,z);

		// Move tree slightly above land height.
		y += 10.0f;

		v[i].Pos  = XMFLOAT3(x,y,z);
		v[i].Size = XMFLOAT2(24.0f, 24.0f);
	}
     
	D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::TreePointSprite) * TreeCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = v;
    HR(device_->CreateBuffer(&vbd, &vinitData, &mTreeSpritesVB));
}

void TreeBillboardApp::DrawTreeSprites(CXMMATRIX viewProj)
{
	Effects::TreeSpriteFX->SetDirLights(mDirLights);
	Effects::TreeSpriteFX->SetEyePosW(mEyePosW);
	Effects::TreeSpriteFX->SetFogColor(Colors::Silver);
	Effects::TreeSpriteFX->SetFogStart(15.0f);
	Effects::TreeSpriteFX->SetFogRange(175.0f);
	Effects::TreeSpriteFX->SetViewProj(viewProj);
	Effects::TreeSpriteFX->SetMaterial(mTreeMat);
	Effects::TreeSpriteFX->SetTreeTextureMapArray(mTreeTextureMapArraySRV);

	immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	immediate_context_->IASetInputLayout(InputLayouts::TreePointSprite);
	UINT stride = sizeof(Vertex::TreePointSprite);
    UINT offset = 0;

	ID3DX11EffectTechnique* treeTech;
	switch(mRenderOptions)
	{
	case RenderOptions::Lighting:
		treeTech = Effects::TreeSpriteFX->Light3Tech;
		break;
	case RenderOptions::Textures:
		treeTech = Effects::TreeSpriteFX->Light3TexAlphaClipTech;
		break;
	case RenderOptions::TexturesAndFog:
		treeTech = Effects::TreeSpriteFX->Light3TexAlphaClipFogTech;
		break;
	}

	D3DX11_TECHNIQUE_DESC techDesc;
	treeTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		immediate_context_->IASetVertexBuffers(0, 1, &mTreeSpritesVB, &stride, &offset);

		float blendFactor[4] = {0.0f, 0.0f, 0.0f, 0.0f};

		if(mAlphaToCoverageOn)
		{
			immediate_context_->OMSetBlendState(RenderStates::AlphaToCoverageBS, blendFactor, 0xffffffff);
		}
		treeTech->GetPassByIndex(p)->Apply(0, immediate_context_);
		immediate_context_->Draw(TreeCount, 0);

		immediate_context_->OMSetBlendState(0, blendFactor, 0xffffffff);
	}
}