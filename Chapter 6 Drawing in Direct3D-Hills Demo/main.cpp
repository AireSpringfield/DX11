///////////////////////////////////////////////////////////////////////////
// Exercise	12: change the struct Vertex or input element description,
// finding that vertex members' order doesn't need to match descriptions'.
// Making sure that offsets are set correctly is just sufficient.
///////////////////////////////////////////////////////////////////////////
// Exercise 13: set and enable scissor rectangle test.
///////////////////////////////////////////////////////////////////////////

#include"d3dapp.h"
#include<fstream>
#include<vector>
#include"d3dx11effect.h"
#include"d3dutility.h"
#include"geometrygenerator.h"

using namespace DirectX;


/*
struct Vertex {
	XMFLOAT3 pos;
	XMFLOAT4 color;
};
*/
struct Vertex {
	XMFLOAT4 color;
	XMFLOAT3 pos;
};

class HillsApp :public D3DApp {
public:
	HillsApp(HINSTANCE h);
	~HillsApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	inline float HeightField(float x, float z);


	void CreateBuffers();
	void CreateFX();
	void CreateInputLayout();
private:
	ID3D11Buffer *vertex_buffer_ = NULL;
	ID3D11Buffer *index_buffer_ = NULL;

	ID3DX11Effect *fx_ = NULL;
	ID3DX11EffectTechnique *fx_tech_ = NULL;
	ID3DX11EffectMatrixVariable *fx_world_view_proj_ = NULL;

	ID3D11InputLayout *input_layout_ = NULL;
	ID3D11RasterizerState *rs_scissor_ = NULL;

	XMFLOAT4X4 world_ = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 view_ = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 proj_ = MathHelper::XMFloat4x4Identity();

	float fov_ = 0.25*XM_PI;
	float near_z_ = 1.0f;
	float far_z_ = 1000.0f;

	float theta_ = 1.5*XM_PI;
	float phi_ = XM_PI / 6.0f;
	float radius_ = 150.0f;

	UINT num_indices_; // Used in DrawScene(), set in CreateBuffers()

	POINT last_mouse_pos_ = { 0, 0 };

};

HillsApp::HillsApp(HINSTANCE hInstance) : D3DApp(hInstance) {
	main_wnd_caption_ = L"Box Demo";
}


HillsApp::~HillsApp() {
	ReleaseCOM(vertex_buffer_);
	ReleaseCOM(index_buffer_);
	ReleaseCOM(fx_);
	ReleaseCOM(input_layout_);
}

void HillsApp::OnResize() {
	D3DApp::OnResize();

	// Update projection matrix
	XMMATRIX proj = XMMatrixPerspectiveFovLH(fov_, AspectRatio(), near_z_, far_z_);
	XMStoreFloat4x4(&proj_, proj);
}

void HillsApp::UpdateScene(float dt) {
	float x = radius_*sinf(phi_)*cosf(theta_);
	float z = radius_*sinf(phi_)*sinf(theta_);
	float y = radius_*cosf(phi_);

	XMVECTOR eye = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR focus = XMVectorZero();

	XMMATRIX view = XMMatrixLookAtLH(eye, focus, up);
	XMStoreFloat4x4(&view_, view);

}

void HillsApp::DrawScene() {

	immediate_context_->ClearRenderTargetView(render_target_view_, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	immediate_context_->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	/************************************************************************************************/
	// Set input assemble, including input layout, primitive topology, vertex buffer, index buffers
	/************************************************************************************************/
	immediate_context_->IASetInputLayout(input_layout_);
	immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT strides[1] = { sizeof(Vertex) };
	UINT offsets[1] = { 0 };
	immediate_context_->IASetVertexBuffers(0, 1, &vertex_buffer_, strides, offsets);
	immediate_context_->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);

	/****************************************************/
	// Set a scissor test rectangle.
	/****************************************************/
	D3D11_RECT rects[1] = { { 200,200,400,400 } };
	immediate_context_->RSSetScissorRects(1, rects);
	immediate_context_->RSSetState(rs_scissor_);

	XMMATRIX world = XMLoadFloat4x4(&world_);
	XMMATRIX view = XMLoadFloat4x4(&view_);
	XMMATRIX proj = XMLoadFloat4x4(&proj_);

	XMMATRIX world_view_proj = world*view*proj;
	fx_world_view_proj_->SetMatrix(reinterpret_cast<float*>(&world_view_proj));
	/**************************************************************************************/
	// Select a technique, apply its each pass to the immediate context and draw primitives
	/**************************************************************************************/
	D3DX11_TECHNIQUE_DESC tech_desc;
	fx_tech_->GetDesc(&tech_desc);
	for (UINT i = 0; i < tech_desc.Passes; ++i) {
		fx_tech_->GetPassByIndex(i)->Apply(0, immediate_context_);

		immediate_context_->DrawIndexed(num_indices_, 0, 0);
	}
	/********************************/
	// Present the swap chain
	/*******************************/
	HR(swap_chain_->Present(0, 0));
}

void HillsApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	last_mouse_pos_.x = x;
	last_mouse_pos_.y = y;

	SetCapture(main_wnd_);
}

void HillsApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void HillsApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - last_mouse_pos_.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - last_mouse_pos_.y));

		// Update angles based on input to orbit camera around box.
		theta_ += dx;
		phi_ += dy;

		// Restrict the angle phi_
		// Be careful: if phi_ is exactly 0, then there will be problem creating view matrix,
		// for we have defined up vector as (0,1,0). Also, when phi_ reaches 0 and take negative
		// values, up/down and left/right will be flipped.
		phi_ = MathHelper::Clamp(phi_, 0.01f, XM_PI/2.0f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		float dx = 0.05f*static_cast<float>(x - last_mouse_pos_.x);
		float dy = 0.05f*static_cast<float>(y - last_mouse_pos_.y);

		// Update the camera radius based on input.
		radius_ += dx - dy;

		// Restrict the radius.
		radius_ = MathHelper::Clamp(radius_, 3.0f, 500.0f);
	}

	last_mouse_pos_.x = x;
	last_mouse_pos_.y = y;
}


bool HillsApp::Init() {
	if (!D3DApp::Init())
		return false;

	CreateBuffers();
	CreateFX();
	CreateInputLayout();

	D3D11_RASTERIZER_DESC rs_scissor_desc;
	rs_scissor_desc.ScissorEnable = true;
	rs_scissor_desc.FillMode = D3D11_FILL_WIREFRAME;
	rs_scissor_desc.CullMode = D3D11_CULL_BACK;
	rs_scissor_desc.DepthClipEnable = true;
	rs_scissor_desc.FrontCounterClockwise = false;
	HR(device_->CreateRasterizerState(&rs_scissor_desc, &rs_scissor_));

	return true;
}

float HillsApp::HeightField(float x, float z) {
	//return 2.0f*(5.0f*x*sinf(0.05f*z) + z*cosf(0.02f*x));
	return 0.3f*(z*sinf(0.1f*x) + x*cosf(0.1f*z));
}

void HillsApp::CreateBuffers() {

	GeometryGenerator::MeshData mesh_data;
	GeometryGenerator generator;

	generator.CreateGrid(150.0f, 150.0f, 150, 150, mesh_data);
	UINT num_vertices = mesh_data.vertices.size();
	UINT num_indices = mesh_data.indices.size();
	num_indices_ = num_indices;
	std::vector<Vertex> vertices(num_vertices);

	for (UINT i = 0; i < vertices.size(); ++i) {
		float x = mesh_data.vertices[i].position.x;
		float z = mesh_data.vertices[i].position.z;
		float y = HeightField(x, z);
		vertices[i].pos = XMFLOAT3(x, y, z);

		// Color the vertex based on its height.
		if (y < -10.0f)
		{
			// Sandy beach color.
			vertices[i].color = XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
		}
		else if (y < 5.0f)
		{
			// Light yellow-green.
			vertices[i].color = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
		}
		else if (y < 12.0f)
		{
			// Dark yellow-green.
			vertices[i].color = XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
		}
		else if (y < 20.0f)
		{
			// Dark brown.
			vertices[i].color = XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
		}
		else
		{
			// White snow.
			vertices[i].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

	// Fill a buffer description
	D3D11_BUFFER_DESC vertex_buffer_desc = {
		sizeof(Vertex) * num_vertices,
		D3D11_USAGE_IMMUTABLE,
		D3D11_BIND_VERTEX_BUFFER,
		0,
		0,
		0
	};
	// Create vertex buffer
	D3D11_SUBRESOURCE_DATA vertex_init_data;
	vertex_init_data.pSysMem = &vertices[0];

	HR(device_->CreateBuffer(&vertex_buffer_desc, &vertex_init_data, &vertex_buffer_));


	// Create index buffer
	
	D3D11_BUFFER_DESC index_buffer_desc;
	index_buffer_desc = {
		sizeof(UINT)*num_indices,
		D3D11_USAGE_IMMUTABLE,
		D3D11_BIND_INDEX_BUFFER,
		0,
		0,
		0
	};


	D3D11_SUBRESOURCE_DATA index_init_data;
	index_init_data.pSysMem = &mesh_data.indices[0];

	HR(device_->CreateBuffer(&index_buffer_desc, &index_init_data, &index_buffer_));

}

void HillsApp::CreateFX() {
	std::ifstream ifs("fx/color.fxo", std::ios::binary);

	// Read from the beginning of compiled file
	ifs.seekg(0, std::ios::end);
	std::streampos size = ifs.tellg();
	std::vector<char> compiled_fx(size);
	ifs.seekg(0, std::ios::beg);
	ifs.read(&compiled_fx[0], size);
	ifs.close();

	HR(D3DX11CreateEffectFromMemory(&compiled_fx[0], size, 0, device_, &fx_));

	fx_tech_ = fx_->GetTechniqueByName("ColorTech");
	fx_world_view_proj_ = fx_->GetVariableByName("g_world_view_proj")->AsMatrix();

}

void HillsApp::CreateInputLayout() {
	D3D11_INPUT_ELEMENT_DESC vertex_element_desc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3DX11_PASS_DESC pass_desc;
	fx_tech_->GetPassByIndex(0)->GetDesc(&pass_desc);
	HR(device_->CreateInputLayout(vertex_element_desc, 2, pass_desc.pIAInputSignature, pass_desc.IAInputSignatureSize,
		&input_layout_));
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showcmd) {
#if defined(DEBUG)||defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	HillsApp app(hInstance);

	if (!app.Init())
		return 0;
	return app.Run();

}