//***************************************************************************************
// LightingDemo.cpp by Frank Luna (C) 2011 All Rights Reserved.
//
// Demonstrates 3D lighting with directional, point, and spot lights.
//
// Controls:
//		Hold the left mouse button down and move the mouse to rotate.
//      Hold the right mouse button down to zoom in and out.
//
//***************************************************************************************


/****************************************************************************/
// Exercise 1: Assign directional light, point light and spot light to be red, green and blue.
/****************************************************************************/
// Exercise 2: Try different specular material's exponent to control shininess.
/****************************************************************************/
// Exercise 4: Use 1 or 2 to control spot light's angle.
/****************************************************************************/

#include"d3dApp.h"
#include"d3dx11effect.h"
#include"geometrygenerator.h"
#include"mathhelper.h"
#include"lighthelper.h"
#include"waves.h"
#include<fstream>

using namespace DirectX;


struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT3 normal;
};

class LightingApp : public D3DApp
{
public:
	LightingApp(HINSTANCE hInstance);
	~LightingApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene(); 

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseWheel(WPARAM wParam, LPARAM lParam);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	float GetHillHeight(float x, float z) const;
	XMFLOAT3 GetHillNormal(float x, float z) const;
	void BuildLandGeometryBuffers();
	void BuildWaveGeometryBuffers();
	void BuildFX();
	void BuildVertexLayout();

private:
	ID3D11Buffer* land_vertex_buffer_ = nullptr;
	ID3D11Buffer* land_index_buffer_ = nullptr;

	ID3D11Buffer* waves_vertex_buffer_ = nullptr;
	ID3D11Buffer* waves_index_buffer_ = nullptr;

	Waves waves_;
	DirectionalLight dir_light_;
	PointLight point_light_;
	SpotLight spot_light_;
	Material land_material_;
	Material waves_material_;

	ID3DX11Effect* fx_ = nullptr;
	ID3DX11EffectTechnique* fx_tech_ = nullptr;
	ID3DX11EffectMatrixVariable* fx_world_view_proj_ = nullptr;
	ID3DX11EffectMatrixVariable* fx_world_ = nullptr;
	ID3DX11EffectMatrixVariable* fx_world_inv_transpose_ = nullptr;
	ID3DX11EffectVectorVariable* fx_eye_pos_world_ = nullptr;
	ID3DX11EffectVariable* fx_dir_light_ = nullptr;
	ID3DX11EffectVariable* fx_point_light_ = nullptr;
	ID3DX11EffectVariable* fx_spot_light_ = nullptr;
	ID3DX11EffectVariable* fx_material_ = nullptr;

	ID3D11InputLayout* input_layout_ = nullptr;

	// Define transformations from local spaces to world space.
	XMFLOAT4X4 land_world_ = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 waves_world_ = MathHelper::XMFloat4x4Identity();

	XMFLOAT4X4 view_ = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 proj_ = MathHelper::XMFloat4x4Identity();

	UINT land_index_cnt_ = 0;

	XMFLOAT3 eye_pos_world_ = XMFLOAT3(0.0f, 0.0f, 0.0f);

	float theta_ = 1.5f*XM_PI;
	float phi_ = XM_PI / 6.0f;
	float radius_ = 15.0f; // radius_ == 0 will lead to error in XMMatrixLookAtLH

	POINT last_mouse_pos_ = { 0,0 };
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	LightingApp app(hInstance);
	
	if( !app.Init() )
		return 0;
	
	return app.Run();
}
 

LightingApp::LightingApp(HINSTANCE hInstance):D3DApp(hInstance){
	main_wnd_caption_ = L"Lighting Demo";

	XMStoreFloat4x4(&waves_world_, XMMatrixTranslation(0.0f, -3.0f, 0.0f));



	// Directional light.
	dir_light_.ambient  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	dir_light_.diffuse  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	dir_light_.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	dir_light_.direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);
 
	// Point light--position is changed every frame to animate in UpdateScene function.
	point_light_.ambient  = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	point_light_.diffuse  = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	point_light_.specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	point_light_.att      = XMFLOAT3(0.0f, 0.1f, 0.0f);
	point_light_.range    = 25.0f;

	// Spot light--position and direction changed every frame to animate in UpdateScene function.
	spot_light_.ambient  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	spot_light_.diffuse  = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	spot_light_.specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	spot_light_.att      = XMFLOAT3(1.0f, 0.0f, 0.0f);
	spot_light_.spot     = 96.0f;
	spot_light_.range    = 10000.0f;


	/*************************/
	// Exercise 1.
	/*************************/
	/*
	// Directional light: red.
	dir_light_.ambient = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	dir_light_.diffuse = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	dir_light_.specular = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

	// Point light: green.
	point_light_.ambient = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	point_light_.diffuse = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	point_light_.specular = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

	// Spot light: blue.
	spot_light_.ambient = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	spot_light_.diffuse = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	spot_light_.specular = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	*/

	/*******************************************************/
	// Exercise 2.
	/*******************************************************/
	land_material_.ambient  = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	land_material_.diffuse  = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	//land_material_.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
	land_material_.specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 64.0f);

	waves_material_.ambient  = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
	waves_material_.diffuse  = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
	//waves_material_.specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 96.0f);
	waves_material_.specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 256.0f);
}

LightingApp::~LightingApp()
{
	ReleaseCOM(land_vertex_buffer_);
	ReleaseCOM(land_index_buffer_);
	ReleaseCOM(waves_vertex_buffer_);
	ReleaseCOM(waves_index_buffer_);

	ReleaseCOM(fx_);
	ReleaseCOM(input_layout_);
}

bool LightingApp::Init()
{
	if(!D3DApp::Init())
		return false;

	waves_.Init(160, 160, 1.0f, 0.03f, 3.25f, 0.4f);

	BuildLandGeometryBuffers();
	BuildWaveGeometryBuffers();
	BuildFX();
	BuildVertexLayout();

	return true;
}

void LightingApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX proj = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&proj_, proj);
}

void LightingApp::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = radius_*sinf(phi_)*cosf(theta_);
	float z = radius_*sinf(phi_)*sinf(theta_);
	float y = radius_*cosf(phi_);

	eye_pos_world_ = XMFLOAT3(x, y, z);

	// Build the view matrix.
	XMVECTOR pos    = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&view_, view);

	//
	// Every quarter second, generate a random wave.
	//
	static float t_base = 0.0f;
	if( (timer_.TotalTime() - t_base) >= 0.25f )
	{
		t_base += 0.25f;
 
		DWORD i = 5 + rand() % (waves_.RowCount()-10);
		DWORD j = 5 + rand() % (waves_.ColumnCount()-10);

		float r = MathHelper::RandF(1.0f, 2.0f);

		waves_.Disturb(i, j, r);
	}

	waves_.Update(dt);

	/*******************************************************/
	// Update the wave vertex buffer with the new solution.
	/*******************************************************/
	
	D3D11_MAPPED_SUBRESOURCE mapped_data_;
	HR(immediate_context_->Map(waves_vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_data_));

	Vertex* v = reinterpret_cast<Vertex*>(mapped_data_.pData);
	for(UINT i = 0; i < waves_.VertexCount(); ++i)
	{
		v[i].pos    = waves_[i];
		v[i].normal = waves_.Normal(i);
	}

	immediate_context_->Unmap(waves_vertex_buffer_, 0);

	//
	// Animate the lights.
	//

	// Circle light over the land surface.
	point_light_.position.x = 70.0f*cosf( 0.2f*timer_.TotalTime() );
	point_light_.position.z = 70.0f*sinf( 0.2f*timer_.TotalTime() );
	point_light_.position.y = MathHelper::Max(GetHillHeight(point_light_.position.x, 
		point_light_.position.z), -3.0f) + 10.0f;


	// The spotlight takes on the camera position and is aimed in the
	// same direction the camera is looking.  In this way, it looks
	// like we are holding a flashlight.
	spot_light_.position = eye_pos_world_;
	XMStoreFloat3(&spot_light_.direction, XMVector3Normalize(target - pos));
	
	/******************************************************/
	// Exercise 4: Use key 1 or 2 to control spot light's angle.
	/******************************************************/
	if (GetAsyncKeyState('1') & 0x8000)
		spot_light_.spot += 5.0f;

	if (GetAsyncKeyState('2') & 0x8000)
		spot_light_.spot -= 5.0f;

}

void LightingApp::DrawScene()
{
	immediate_context_->ClearRenderTargetView(render_target_view_, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	immediate_context_->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	immediate_context_->IASetInputLayout(input_layout_);
    immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
 
	UINT stride = sizeof(Vertex);
    UINT offset = 0;

	XMMATRIX view  = XMLoadFloat4x4(&view_);
	XMMATRIX proj  = XMLoadFloat4x4(&proj_);
	XMMATRIX view_proj = view*proj;

	// Set per frame constants in Effect.
	fx_dir_light_->SetRawValue(&dir_light_, 0, sizeof(dir_light_));
	fx_point_light_->SetRawValue(&point_light_, 0, sizeof(point_light_));
	fx_spot_light_->SetRawValue(&spot_light_, 0, sizeof(spot_light_));
	fx_eye_pos_world_->SetRawValue(&eye_pos_world_, 0, sizeof(eye_pos_world_));
 
    D3DX11_TECHNIQUE_DESC tech_desc;
    fx_tech_->GetDesc( &tech_desc );
    for(UINT p = 0; p < tech_desc.Passes; ++p)
    {
		/***********************/
		// Draw the hills.
		/***********************/
		immediate_context_->IASetVertexBuffers(0, 1, &land_vertex_buffer_, &stride, &offset);
		immediate_context_->IASetIndexBuffer(land_index_buffer_, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&land_world_);
		XMMATRIX world_inv_transpose = MathHelper::InverseTranspose(world);
		XMMATRIX world_view_proj = world*view*proj;
		
		fx_world_->SetMatrix(reinterpret_cast<float*>(&world));
		fx_world_inv_transpose_->SetMatrix(reinterpret_cast<float*>(&world_inv_transpose));
		fx_world_view_proj_->SetMatrix(reinterpret_cast<float*>(&world_view_proj));
		fx_material_->SetRawValue(&land_material_, 0, sizeof(land_material_));

		fx_tech_->GetPassByIndex(p)->Apply(0, immediate_context_);
		immediate_context_->DrawIndexed(land_index_cnt_, 0, 0);

		/***********************************/
		// Draw the waves.
		/***********************************/
		immediate_context_->IASetVertexBuffers(0, 1, &waves_vertex_buffer_, &stride, &offset);
		immediate_context_->IASetIndexBuffer(waves_index_buffer_, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		world = XMLoadFloat4x4(&waves_world_);
		world_inv_transpose = MathHelper::InverseTranspose(world);
		world_view_proj = world*view*proj;
		
		fx_world_->SetMatrix(reinterpret_cast<float*>(&world));
		fx_world_inv_transpose_->SetMatrix(reinterpret_cast<float*>(&world_inv_transpose));
		fx_world_view_proj_->SetMatrix(reinterpret_cast<float*>(&world_view_proj));
		fx_material_->SetRawValue(&waves_material_, 0, sizeof(waves_material_));;

		fx_tech_->GetPassByIndex(p)->Apply(0, immediate_context_);
		immediate_context_->DrawIndexed(3*waves_.TriangleCount(), 0, 0);
    }

	HR(swap_chain_->Present(0, 0));
}

void LightingApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	last_mouse_pos_.x = x;
	last_mouse_pos_.y = y;

	SetCapture(main_wnd_);
}

void LightingApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void LightingApp::OnMouseWheel(WPARAM wParam, LPARAM lParam) {
	short delta = GET_WHEEL_DELTA_WPARAM(wParam);
	int inc = -delta / WHEEL_DELTA;
	radius_ += inc*5.0f;
	radius_ = MathHelper::Clamp(radius_, 0.5f, 100.0f);
}

void LightingApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if( (btnState & MK_LBUTTON) != 0 )
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - last_mouse_pos_.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - last_mouse_pos_.y));

		// Update angles based on input to orbit camera around box.
		theta_ += dx;
		phi_  += dy;

		// Restrict the angle mPhi.
		phi_ = MathHelper::Clamp(phi_, 0.1f, MathHelper::Pi-0.1f);
	}
	else if( (btnState & MK_RBUTTON) != 0 )
	{
		// Make each pixel correspond to 0.2 unit in the scene.
		float dx = 0.2f*static_cast<float>(x - last_mouse_pos_.x);
		float dy = 0.2f*static_cast<float>(y - last_mouse_pos_.y);

		// Update the camera radius based on input.
		radius_ += dx - dy;

		// Restrict the radius.
		radius_ = MathHelper::Clamp(radius_, 50.0f, 500.0f);
	}

	last_mouse_pos_.x = x;
	last_mouse_pos_.y = y;
}

float LightingApp::GetHillHeight(float x, float z) const
{
	return 0.3f*( z*sinf(0.1f*x) + x*cosf(0.1f*z) );
}

XMFLOAT3 LightingApp::GetHillNormal(float x, float z) const
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

void LightingApp::BuildLandGeometryBuffers()
{
	GeometryGenerator::MeshData grid;
 
	GeometryGenerator geo_gen;

	geo_gen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

	land_index_cnt_ = grid.indices.size();

	//
	// Extract the vertex elements we are interested and apply the height function to
	// each vertex.  
	//

	std::vector<Vertex> vertices(grid.vertices.size());
	for(size_t i = 0; i < grid.vertices.size(); ++i)
	{
		XMFLOAT3 pos = grid.vertices[i].position;

		pos.y = GetHillHeight(pos.x, pos.z);
		
		vertices[i].pos    = pos;
		vertices[i].normal = GetHillNormal(pos.x, pos.z);
	}

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * grid.vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vertex_init_data;
    vertex_init_data.pSysMem = &vertices[0];
    HR(device_->CreateBuffer(&vbd, &vertex_init_data, &land_vertex_buffer_));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * land_index_cnt_;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA index_init_data;
	index_init_data.pSysMem = &grid.indices[0];
    HR(device_->CreateBuffer(&ibd, &index_init_data, &land_index_buffer_));
}

void LightingApp::BuildWaveGeometryBuffers()
{
	// Create the vertex buffer.  Note that we allocate space only, as
	// we will be updating the data every time step of the simulation.

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex) * waves_.VertexCount();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vbd.MiscFlags = 0;
    HR(device_->CreateBuffer(&vbd, nullptr, &waves_vertex_buffer_));


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
    D3D11_SUBRESOURCE_DATA index_init_data;
    index_init_data.pSysMem = &indices[0];
    HR(device_->CreateBuffer(&ibd, &index_init_data, &waves_index_buffer_));
}

void LightingApp::BuildFX()
{
	std::ifstream fin("FX/lighting.fxo", std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);

	fin.read(&compiledShader[0], size);
	fin.close();
	
	HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size, 
		0, device_, &fx_));

	fx_tech_ = fx_->GetTechniqueByName("LightTech");
	fx_world_view_proj_ = fx_->GetVariableByName("gWorldViewProj")->AsMatrix();
	fx_world_ = fx_->GetVariableByName("gWorld")->AsMatrix();
	fx_world_inv_transpose_ = fx_->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	fx_eye_pos_world_ = fx_->GetVariableByName("gEyePosW")->AsVector();
	fx_dir_light_ = fx_->GetVariableByName("gDirLight");
	fx_point_light_ = fx_->GetVariableByName("gPointLight");
	fx_spot_light_ = fx_->GetVariableByName("gSpotLight");
	fx_material_ = fx_->GetVariableByName("gMaterial");
}

void LightingApp::BuildVertexLayout()
{
	// Create the vertex input layout.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// Create the input layout
    D3DX11_PASS_DESC passDesc;
    fx_tech_->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device_->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature, 
		passDesc.IAInputSignatureSize, &input_layout_));
}