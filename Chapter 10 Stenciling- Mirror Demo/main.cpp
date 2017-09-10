//***************************************************************************************
// MirrorDemo.cpp by Frank Luna (C) 2011 All Rights Reserved.
//
// Demonstrates using the stencil buffer to mask out areas from being drawn to, 
// and to prevent "double blending."
//
// Controls:
//		Hold the left mouse button down and move the mouse to rotate.
//      Hold the right mouse button down to zoom in and out.
//
//      Press '1' - Lighting only render mode.
//      Press '2' - Texture render mode.
//      Press '3' - Fog render mode.
//
//		Move the skull left/right/up/down with 'A'/'D'/'W'/'S' keys.
//
//***************************************************************************************

#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "geometrygenerator.h"
#include "mathhelper.h"
#include "lighthelper.h"
#include "effects.h"
#include "vertex.h"
#include "renderStates.h"
#include "waves.h"


using namespace DirectX;

enum RenderOptions
{
	Lighting = 0,
	Textures = 1,
	TexturesAndFog = 2
};

class MirrorApp : public D3DApp
{
public:
	MirrorApp(HINSTANCE hInstance);
	~MirrorApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene(); 

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildRoomGeometryBuffers();
	void BuildSkullGeometryBuffers();

private:
	ID3D11Buffer* RoomVB_ = nullptr;

	ID3D11Buffer* SkullVB_ = nullptr;
	ID3D11Buffer* SkullIB_ = nullptr;

	ID3D11ShaderResourceView* floorDiffuseMapSRV_ = nullptr;
	ID3D11ShaderResourceView* wallDiffuseMapSRV_ = nullptr;
	ID3D11ShaderResourceView* mirrorDiffuseMapSRV_ = nullptr;

	DirectionalLight dirLights_[3];
	Material roomMaterial_;
	Material skullMaterial_;
	Material mirrorMaterial_;
	Material shadowMaterial_;

	XMFLOAT4X4 roomWorld_ = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 skullWorld_ = MathHelper::XMFloat4x4Identity();

	UINT skullIndexCnt_ = 0;
	XMFLOAT3 skullTranslation_ = XMFLOAT3(0.0f, 1.0f, -5.0f);

	XMFLOAT4X4 view_ = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 proj_ = MathHelper::XMFloat4x4Identity();

	RenderOptions renderOptions_ = Textures;
	
	XMFLOAT3 eyePosW_ = XMFLOAT3(0.0f, 0.0f, 0.0f);

	float theta_ = 1.5f*MathHelper::Pi;
	float phi_ = 0.5f*MathHelper::Pi;
	float radius_ = 5.0f;

	POINT lastMousePos_ = { 0,0 };
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	MirrorApp theApp(hInstance);
	
	if( !theApp.Init() )
		return 0;
	
	return theApp.Run();
}

MirrorApp::MirrorApp(HINSTANCE hInstance)
: D3DApp(hInstance)
{
	main_wnd_caption_ = L"Mirror Demo";
	enable_msaa4x_ = false;



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

	roomMaterial_.ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	roomMaterial_.diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	roomMaterial_.specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	skullMaterial_.ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	skullMaterial_.diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	skullMaterial_.specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	// Reflected material is transparent so it blends into mirror.
	mirrorMaterial_.ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mirrorMaterial_.diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	mirrorMaterial_.specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	shadowMaterial_.ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	shadowMaterial_.diffuse  = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
	shadowMaterial_.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
}

MirrorApp::~MirrorApp()
{
	immediate_context_->ClearState();
	ReleaseCOM(RoomVB_);
	ReleaseCOM(SkullVB_);
	ReleaseCOM(SkullIB_);
	ReleaseCOM(floorDiffuseMapSRV_);
	ReleaseCOM(wallDiffuseMapSRV_);
	ReleaseCOM(mirrorDiffuseMapSRV_);
 
	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool MirrorApp::Init()
{
	if(!D3DApp::Init())
		return false;
 
	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(device_);
	InputLayouts::InitAll(device_);
	RenderStates::InitAll(device_);

	CreateDDSShaderResourceViewFromFile(device_, L"Textures/checkboard.dds", &floorDiffuseMapSRV_);

	CreateDDSShaderResourceViewFromFile(device_, L"Textures/brick01.dds",&wallDiffuseMapSRV_);

	CreateDDSShaderResourceViewFromFile(device_, L"Textures/ice.dds",  &mirrorDiffuseMapSRV_);
 
	BuildRoomGeometryBuffers();
	BuildSkullGeometryBuffers();

	return true;
}

void MirrorApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&proj_, P);
}

void MirrorApp::UpdateScene(float dt)
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
	// Switch the render mode based in key input.
	//
	if( GetAsyncKeyState('1') & 0x8000 )
		renderOptions_ = RenderOptions::Lighting; 

	if( GetAsyncKeyState('2') & 0x8000 )
		renderOptions_ = RenderOptions::Textures; 

	if( GetAsyncKeyState('3') & 0x8000 )
		renderOptions_ = RenderOptions::TexturesAndFog; 


	//
	// Allow user to move box.
	//
	if( GetAsyncKeyState('A') & 0x8000 )
		skullTranslation_.x -= 1.0f*dt;

	if( GetAsyncKeyState('D') & 0x8000 )
		skullTranslation_.x += 1.0f*dt;

	if( GetAsyncKeyState('W') & 0x8000 )
		skullTranslation_.y += 1.0f*dt;

	if( GetAsyncKeyState('S') & 0x8000 )
		skullTranslation_.y -= 1.0f*dt;

	// Don't let user move below ground plane.
	skullTranslation_.y = MathHelper::Max(skullTranslation_.y, 0.0f);

	// Update the new world matrix.
	XMMATRIX skullRotate = XMMatrixRotationY(0.5f*MathHelper::Pi);
	XMMATRIX skullScale = XMMatrixScaling(0.45f, 0.45f, 0.45f);
	XMMATRIX skullOffset = XMMatrixTranslation(skullTranslation_.x, skullTranslation_.y, skullTranslation_.z);
	XMStoreFloat4x4(&skullWorld_, skullRotate*skullScale*skullOffset);
}

void MirrorApp::DrawScene()
{
	immediate_context_->ClearRenderTargetView(render_target_view_, reinterpret_cast<const float*>(&Colors::Black));
	immediate_context_->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	immediate_context_->IASetInputLayout(InputLayouts::Basic32);
    immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
 
	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};

	UINT stride = sizeof(Vertex::Basic32);
    UINT offset = 0;
 
	XMMATRIX view  = XMLoadFloat4x4(&view_);
	XMMATRIX proj  = XMLoadFloat4x4(&proj_);
	XMMATRIX viewProj = view*proj;

	// Set per frame constants.
	Effects::BasicFX->SetDirLights(dirLights_);
	Effects::BasicFX->SetEyePosW(eyePosW_);
	Effects::BasicFX->SetFogColor(Colors::Black);
	Effects::BasicFX->SetFogStart(2.0f);
	Effects::BasicFX->SetFogRange(40.0f);
 
	// Skull doesn't have texture coordinates, so we can't texture it.
	ID3DX11EffectTechnique* activeTech;
	ID3DX11EffectTechnique* activeSkullTech;

	switch(renderOptions_)
	{
	case RenderOptions::Lighting:
		activeTech = Effects::BasicFX->Light3Tech;
		activeSkullTech = Effects::BasicFX->Light3Tech;
		break;
	case RenderOptions::Textures:
		activeTech = Effects::BasicFX->Light3TexTech;
		activeSkullTech = Effects::BasicFX->Light3Tech;
		break;
	case RenderOptions::TexturesAndFog:
		activeTech = Effects::BasicFX->Light3TexFogTech;
		activeSkullTech = Effects::BasicFX->Light3FogTech;
		break;
	}

	D3DX11_TECHNIQUE_DESC techDesc;

	/**************************************************************/
	// 1. Draw the floor and walls to the back buffer as normal.
	/**************************************************************/
	
	activeTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		ID3DX11EffectPass* pass = activeTech->GetPassByIndex( p );

		immediate_context_->IASetVertexBuffers(0, 1, &RoomVB_, &stride, &offset);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&roomWorld_);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;
		
		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(roomMaterial_);

		// Floor
		Effects::BasicFX->SetDiffuseMap(floorDiffuseMapSRV_);
		pass->Apply(0, immediate_context_);
		immediate_context_->Draw(6, 0);

		// Wall
		Effects::BasicFX->SetDiffuseMap(wallDiffuseMapSRV_);
		pass->Apply(0, immediate_context_);
		immediate_context_->Draw(18, 6);
	}

	/***************************************************/
	// 2. Draw the skull to the back buffer as normal.
	/***************************************************/

	activeSkullTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		ID3DX11EffectPass* pass = activeSkullTech->GetPassByIndex( p );

		immediate_context_->IASetVertexBuffers(0, 1, &SkullVB_, &stride, &offset);
		immediate_context_->IASetIndexBuffer(SkullIB_, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&skullWorld_);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetMaterial(skullMaterial_);

		pass->Apply(0, immediate_context_);
		immediate_context_->DrawIndexed(skullIndexCnt_, 0, 0);
	}

	/************************************************/
	// 3. Draw the mirror to stencil buffer only.
	/************************************************/

	activeTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		ID3DX11EffectPass* pass = activeTech->GetPassByIndex( p );

		immediate_context_->IASetVertexBuffers(0, 1, &RoomVB_, &stride, &offset);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&roomWorld_);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;
		
		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());

		// Do not write to render target.
		immediate_context_->OMSetBlendState(RenderStates::NoRenderTargetWritesBS, blendFactor, 0xffffffff);

		// Render visible mirror pixels to stencil buffer.
		// Do not write mirror depth to depth buffer at this point, otherwise it will occlude the reflection.

		// Because we do not have a wall behind the mirror. However, if so, when marking the mirror in stencil
		// buffer, we need to set the depth that are larger than mirror's pixel to 1.0, otherwise that wall
		// behind the mirror will occlude our reflection.
		immediate_context_->OMSetDepthStencilState(RenderStates::MarkMirrorDSS, 1);
		// if(StencilRef & StencilReadMask "op" Value & StencilReadMask)
		//		accept pixel
		// else
		//		reject pixel


		pass->Apply(0, immediate_context_);
		immediate_context_->Draw(6, 24);

		// Restore states.
		immediate_context_->OMSetDepthStencilState(0, 0);
		immediate_context_->OMSetBlendState(0, blendFactor, 0xffffffff);
	}


	/*******************************/
	// 4. Draw the skull reflection.
	/*******************************/
	activeSkullTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		ID3DX11EffectPass* pass = activeSkullTech->GetPassByIndex( p );

		immediate_context_->IASetVertexBuffers(0, 1, &SkullVB_, &stride, &offset);
		immediate_context_->IASetIndexBuffer(SkullIB_, DXGI_FORMAT_R32_UINT, 0);

		XMVECTOR mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // xy plane (px, py, pz, d) where d = -p0*n
		XMMATRIX R = XMMatrixReflect(mirrorPlane);
		XMMATRIX world = XMLoadFloat4x4(&skullWorld_) * R;
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetMaterial(skullMaterial_);

		// Cache the old light directions, and reflect the light directions.
		XMFLOAT3 oldLightdirections[3];
		for(int i = 0; i < 3; ++i)
		{
			oldLightdirections[i] = dirLights_[i].direction;
			
			XMVECTOR lightDir = XMLoadFloat3(&dirLights_[i].direction);
			XMVECTOR reflectedLightDir = XMVector3TransformNormal(lightDir, R);
			XMStoreFloat3(&dirLights_[i].direction, reflectedLightDir);
		}

		Effects::BasicFX->SetDirLights(dirLights_);

		// Cull clockwise triangles for reflection.
		immediate_context_->RSSetState(RenderStates::CullClockwiseRS);

		// Only draw reflection into visible mirror pixels as marked by the stencil buffer. 
		immediate_context_->OMSetDepthStencilState(RenderStates::DrawReflectionDSS, 1);

		// For exercise 3.
		immediate_context_->OMSetDepthStencilState(0, 0);


		pass->Apply(0, immediate_context_);
		immediate_context_->DrawIndexed(skullIndexCnt_, 0, 0);

		// Restore default states.
		immediate_context_->RSSetState(0);	
		immediate_context_->OMSetDepthStencilState(0, 0);	

		// Restore light directions.
		for(int i = 0; i < 3; ++i)
		{
			dirLights_[i].direction = oldLightdirections[i];
		}

		Effects::BasicFX->SetDirLights(dirLights_);
	}

	/*************************************************************************/
	// 5. Draw the mirror to the back buffer as usual but with transparency
	// blending so the reflection shows through.
	/*************************************************************************/ 

	activeTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		ID3DX11EffectPass* pass = activeTech->GetPassByIndex( p );

		immediate_context_->IASetVertexBuffers(0, 1, &RoomVB_, &stride, &offset);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&roomWorld_);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;
		
		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(mirrorMaterial_);
		Effects::BasicFX->SetDiffuseMap(mirrorDiffuseMapSRV_);

		// Mirror
		immediate_context_->OMSetBlendState(RenderStates::TransparentBS, blendFactor, 0xffffffff);
		pass->Apply(0, immediate_context_);
		immediate_context_->Draw(6, 24);
	}

	/**************************************/
	// 6. Draw the skull shadow.
	/**************************************/
	activeSkullTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		ID3DX11EffectPass* pass = activeSkullTech->GetPassByIndex( p );

		immediate_context_->IASetVertexBuffers(0, 1, &SkullVB_, &stride, &offset);
		immediate_context_->IASetIndexBuffer(SkullIB_, DXGI_FORMAT_R32_UINT, 0);

		XMVECTOR shadowPlane = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // xz plane
		XMVECTOR toMainLight = -XMLoadFloat3(&dirLights_[0].direction);
		XMMATRIX S =  XMMatrixShadow(shadowPlane, toMainLight);
		XMMATRIX shadowOffsetY = XMMatrixTranslation(0.0f, 0.001f, 0.0f);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&skullWorld_)*S*shadowOffsetY;
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;
		
		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetMaterial(shadowMaterial_);


		// Here's a problem: we cannot cast the shadow to the mirror, as pixels overlapped by the mirror
		// already have a stencil value 1.
		// But no need to worry, because we cast the shadow only to the floor, not the wall...
	

		immediate_context_->OMSetDepthStencilState(RenderStates::NoDoubleBlendDSS, 0);
		// Blend state is still TransparentBS.

		// For exercise 4.
		immediate_context_->OMSetDepthStencilState(0, 0);

		pass->Apply(0, immediate_context_);
		immediate_context_->DrawIndexed(skullIndexCnt_, 0, 0);

		// Restore default states.
		immediate_context_->OMSetBlendState(0, blendFactor, 0xffffffff);
		immediate_context_->OMSetDepthStencilState(0, 0);
	}



	// For exercise 5.
	immediate_context_->ClearRenderTargetView(render_target_view_, reinterpret_cast<const float*>(&Colors::Black));
	immediate_context_->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Draw the wall.	

	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

		immediate_context_->IASetVertexBuffers(0, 1, &RoomVB_, &stride, &offset);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&roomWorld_);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(roomMaterial_);

		D3D11_DEPTH_STENCIL_DESC desc = { 0 };
		ID3D11DepthStencilState *state = nullptr;
		desc.DepthEnable = false;
		desc.DepthEnable = true;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_LESS;

		device_->CreateDepthStencilState(&desc, &state);
		immediate_context_->OMSetDepthStencilState(state, 0);

		// Wall
		Effects::BasicFX->SetDiffuseMap(wallDiffuseMapSRV_);
		pass->Apply(0, immediate_context_);
		immediate_context_->Draw(18, 6);
	}


	// Draw the skull behind the wall.
	activeSkullTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeSkullTech->GetPassByIndex(p);

		immediate_context_->IASetVertexBuffers(0, 1, &SkullVB_, &stride, &offset);
		immediate_context_->IASetIndexBuffer(SkullIB_, DXGI_FORMAT_R32_UINT, 0);

		XMVECTOR mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // xy plane (px, py, pz, d) where d = -p0*n
		XMMATRIX R = XMMatrixReflect(mirrorPlane);
		XMMATRIX world = XMLoadFloat4x4(&skullWorld_) * R;
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetMaterial(skullMaterial_);

		// Cache the old light directions, and reflect the light directions.
		XMFLOAT3 oldLightdirections[3];
		for (int i = 0; i < 3; ++i)
		{
			oldLightdirections[i] = dirLights_[i].direction;

			XMVECTOR lightDir = XMLoadFloat3(&dirLights_[i].direction);
			XMVECTOR reflectedLightDir = XMVector3TransformNormal(lightDir, R);
			XMStoreFloat3(&dirLights_[i].direction, reflectedLightDir);
		}

		Effects::BasicFX->SetDirLights(dirLights_);

		// Cull clockwise triangles for reflection.
		immediate_context_->RSSetState(RenderStates::CullClockwiseRS);


		// Depth stencil state assigned in exercise 5.
		D3D11_DEPTH_STENCIL_DESC desc = { 0 };
		ID3D11DepthStencilState *state = nullptr;
		desc.DepthEnable = true;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_LESS;

		device_->CreateDepthStencilState(&desc, &state);
		immediate_context_->OMSetDepthStencilState(state, 0);;



		pass->Apply(0, immediate_context_);
		immediate_context_->DrawIndexed(skullIndexCnt_, 0, 0);

		// Restore default states.
		immediate_context_->RSSetState(0);
		immediate_context_->OMSetDepthStencilState(0, 0);

		// Restore light directions.
		for (int i = 0; i < 3; ++i)
		{
			dirLights_[i].direction = oldLightdirections[i];
		}

		Effects::BasicFX->SetDirLights(dirLights_);
	}





	HR(swap_chain_->Present(0, 0));
}

void MirrorApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	lastMousePos_.x = x;
	lastMousePos_.y = y;

	SetCapture(main_wnd_);
}

void MirrorApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void MirrorApp::OnMouseMove(WPARAM btnState, int x, int y)
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
		float dx = 0.01f*static_cast<float>(x - lastMousePos_.x);
		float dy = 0.01f*static_cast<float>(y - lastMousePos_.y);

		// Update the camera radius based on input.
		radius_ += dx - dy;

		// Restrict the radius.
		radius_ = MathHelper::Clamp(radius_, 3.0f, 50.0f);
	}

	lastMousePos_.x = x;
	lastMousePos_.y = y;
}

void MirrorApp::BuildRoomGeometryBuffers()
{
	// Create and specify geometry.  For this sample we draw a floor
	// and a wall with a mirror on it.  We put the floor, wall, and
	// mirror geometry in one vertex buffer.
	//
	//   |--------------|
	//   |    Wall      |
    //   |----|----|----|
    //   |Wall|Mirr|Wall|
	//   |    | or |    |
    //   /--------------/
    //  /   Floor      /
	// /--------------/

 
	Vertex::Basic32 v[30];

	// Floor: Observe we tile texture coordinates.
	v[0] = Vertex::Basic32(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f);
	v[1] = Vertex::Basic32(-3.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	v[2] = Vertex::Basic32( 7.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f);
	
	v[3] = Vertex::Basic32(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f);
	v[4] = Vertex::Basic32( 7.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f);
	v[5] = Vertex::Basic32( 7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 4.0f, 4.0f);

	// Wall: Observe we tile texture coordinates, and that we
	// leave a gap in the middle for the mirror.
	v[6]  = Vertex::Basic32(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[7]  = Vertex::Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[8]  = Vertex::Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f);
	
	v[9]  = Vertex::Basic32(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[10] = Vertex::Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f);
	v[11] = Vertex::Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 2.0f);

	v[12] = Vertex::Basic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[13] = Vertex::Basic32(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[14] = Vertex::Basic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f);
	
	v[15] = Vertex::Basic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[16] = Vertex::Basic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f);
	v[17] = Vertex::Basic32(7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 2.0f);

	v[18] = Vertex::Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[19] = Vertex::Basic32(-3.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[20] = Vertex::Basic32( 7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f);
	
	v[21] = Vertex::Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[22] = Vertex::Basic32( 7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f);
	v[23] = Vertex::Basic32( 7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 1.0f);

	// Mirror
	v[24] = Vertex::Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[25] = Vertex::Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[26] = Vertex::Basic32( 2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	
	v[27] = Vertex::Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[28] = Vertex::Basic32( 2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[29] = Vertex::Basic32( 2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * 30;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = v;
    HR(device_->CreateBuffer(&vbd, &vinitData, &RoomVB_));
}

void MirrorApp::BuildSkullGeometryBuffers()
{
	std::ifstream fin("Models/skull.txt");
	
	if(!fin)
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
	
	std::vector<Vertex::Basic32> vertices(vcount);
	for(UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	skullIndexCnt_ = 3*tcount;
	std::vector<UINT> indices(skullIndexCnt_);
	for(UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i*3+0] >> indices[i*3+1] >> indices[i*3+2];
	}

	fin.close();

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * vcount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(device_->CreateBuffer(&vbd, &vinitData, &SkullVB_));

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
    HR(device_->CreateBuffer(&ibd, &iinitData, &SkullIB_));
}