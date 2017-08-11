////////////////////////////////////////////////////////////////////////
// Exercise 2: use two vertex buffers and two input slots.
////////////////////////////////////////////////////////////////////////
// Exercise 6: distort the vertices with sine function in vertex shader.
////////////////////////////////////////////////////////////////////////

#include"d3dapp.h"
#include<fstream>
#include<vector>
#include"mathhelper.h"
#include"d3dx11effect.h"


using namespace DirectX;


typedef	XMFLOAT3 Pos;
typedef	XMFLOAT4 Color;


class BoxApp :public D3DApp {
public:
	BoxApp(HINSTANCE h);
	~BoxApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void CreateBuffers();
	void CreateFX();
	void CreateInputLayout();
private:
	//ID3D11Buffer *vertex_buffer_ = NULL;
	
	// Buffer 0 for position, Buffer 1 for color.
	ID3D11Buffer *vertex_buffers_[2] = { NULL, NULL };
	ID3D11Buffer *index_buffer_ = NULL;

	ID3DX11Effect *fx_ = NULL;
	ID3DX11EffectTechnique *fx_tech_ = NULL;
	ID3DX11EffectMatrixVariable *fx_world_view_proj_ = NULL;
	ID3DX11EffectScalarVariable *fx_time_ = NULL;

	ID3D11InputLayout *input_layout_ = NULL;

	XMFLOAT4X4 world_ = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 view_ = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 proj_ = MathHelper::XMFloat4x4Identity();

	float fov_ = 0.25*XM_PI;
	float near_z_ = 1.0f;
	float far_z_ = 1000.0f;

	float theta_ = 0.0f;
	float phi_ = 0.25*XM_PI;
	float radius_ = 5.0f;

	POINT last_mouse_pos_ = { 0, 0 };

};

BoxApp::BoxApp(HINSTANCE hInstance) : D3DApp(hInstance){
	main_wnd_caption_ = L"Box Demo";
}


BoxApp::~BoxApp() {
	ReleaseCOM(vertex_buffers_[0]);
	ReleaseCOM(vertex_buffers_[1]);
	ReleaseCOM(index_buffer_);
	ReleaseCOM(fx_);
	ReleaseCOM(input_layout_);
	
}

void BoxApp::OnResize() {
	D3DApp::OnResize();

	// Update projection matrix
	XMMATRIX proj = XMMatrixPerspectiveFovLH(fov_, AspectRatio(), near_z_, far_z_);
	XMStoreFloat4x4(&proj_, proj);
}

void BoxApp::UpdateScene(float dt) {
	float x = radius_*sinf(phi_)*cosf(theta_);
	float z = radius_*sinf(phi_)*sinf(theta_);
	float y = radius_*cosf(phi_);

	XMVECTOR eye = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR focus = XMVectorZero();

	XMMATRIX view = XMMatrixLookAtLH(eye, focus, up);
	XMStoreFloat4x4(&view_, view);

}

void BoxApp::DrawScene() {
	
	immediate_context_->ClearRenderTargetView(render_target_view_, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	immediate_context_->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	/************************************************************************************************/
	// Set input assemble, including input layout, primitive topology, vertex buffer, index buffers
	/************************************************************************************************/
	immediate_context_->IASetInputLayout(input_layout_);
	immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT strides[2] = { sizeof(Pos), sizeof(Color) };
	UINT offsets[2] = { 0, 0 };
	immediate_context_->IASetVertexBuffers(0, 2, vertex_buffers_, strides, offsets);
	immediate_context_->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);

	

	XMMATRIX world = XMLoadFloat4x4(&world_);
	XMMATRIX view = XMLoadFloat4x4(&view_);
	XMMATRIX proj = XMLoadFloat4x4(&proj_);

	XMMATRIX world_view_proj = world*view*proj;
	fx_world_view_proj_->SetMatrix(reinterpret_cast<float*>(&world_view_proj));
	fx_time_->SetFloat(timer_.TotalTime());
	/**************************************************************************************/
	// Select a technique, apply its each pass to the immediate context and draw primitives
	/**************************************************************************************/
	D3DX11_TECHNIQUE_DESC tech_desc;
	fx_tech_->GetDesc(&tech_desc);
	for (UINT i = 0; i < tech_desc.Passes; ++i) {
		fx_tech_->GetPassByIndex(i)->Apply(0, immediate_context_);

		immediate_context_->DrawIndexed(36, 0, 0);
	}
	/********************************/
	// Present the swap chain
	/*******************************/
	HR(swap_chain_->Present(0, 0));
}

void BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	last_mouse_pos_.x = x;
	last_mouse_pos_.y = y;

	SetCapture(main_wnd_);
}

void BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - last_mouse_pos_.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - last_mouse_pos_.y));

		// Update angles based on input to orbit camera around box.
		theta_ += dx;
		phi_ += dy;

		// Restrict the angle mPhi.
		phi_ = MathHelper::Clamp(phi_, 0.1f, XM_PI - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		float dx = 0.005f*static_cast<float>(x - last_mouse_pos_.x);
		float dy = 0.005f*static_cast<float>(y - last_mouse_pos_.y);

		// Update the camera radius based on input.
		radius_ += dx - dy;

		// Restrict the radius.
		radius_ = MathHelper::Clamp(radius_, 3.0f, 15.0f);
	}

	last_mouse_pos_.x = x;
	last_mouse_pos_.y = y;
}


bool BoxApp::Init() {
	if (!D3DApp::Init())
		return false;

	CreateBuffers();
	CreateFX();
	CreateInputLayout();

	return true;
}



void BoxApp::CreateBuffers() {

	// Vertex data for initialize immutable vertex buffer
	Pos positions[] =
	{
		 XMFLOAT3(-1.0f, -1.0f, -1.0f),
		 XMFLOAT3(-1.0f, +1.0f, -1.0f),
		 XMFLOAT3(+1.0f, +1.0f, -1.0f),
		 XMFLOAT3(+1.0f, -1.0f, -1.0f),
		 XMFLOAT3(-1.0f, -1.0f, +1.0f),
		 XMFLOAT3(-1.0f, +1.0f, +1.0f),
		 XMFLOAT3(+1.0f, +1.0f, +1.0f),
		 XMFLOAT3(+1.0f, -1.0f, +1.0f),
	};

	Color colors[] = {
		XMFLOAT4((const float*)&Colors::White) ,
		XMFLOAT4((const float*)&Colors::Black),
		XMFLOAT4((const float*)&Colors::Red) ,
		XMFLOAT4((const float*)&Colors::Green),
		XMFLOAT4((const float*)&Colors::Blue) ,
		XMFLOAT4((const float*)&Colors::Yellow) ,
		XMFLOAT4((const float*)&Colors::Cyan) ,
		XMFLOAT4((const float*)&Colors::Magenta)
	};

	D3D11_BUFFER_DESC pos_buffer_desc = {
		sizeof(Pos) * 8,
		D3D11_USAGE_IMMUTABLE,
		D3D11_BIND_VERTEX_BUFFER,
		0,
		0,
		0,
	};

	D3D11_BUFFER_DESC color_buffer_desc = {
		sizeof(Color) * 8,
		D3D11_USAGE_IMMUTABLE,
		D3D11_BIND_VERTEX_BUFFER,
		0,
		0,
		0,
	};

	D3D11_SUBRESOURCE_DATA vertex_init_data;
	vertex_init_data.pSysMem = &positions[0];
	HR(device_->CreateBuffer(&pos_buffer_desc, &vertex_init_data, &vertex_buffers_[0]));

	vertex_init_data.pSysMem = &colors[0];
	HR(device_->CreateBuffer(&color_buffer_desc, &vertex_init_data, &vertex_buffers_[1]));



	// Create index buffer
	// Triangle faces
	UINT indices[] = {
		
		// Wrong index winding order, illustrating backward culling
		/*
		// Front
		0, 1, 2,
		0, 2, 3,
		// Back
		4, 5, 6,
		4, 6, 7,
		// Left
		0, 1, 4,
		0, 4, 5,
		// Right
		2, 3, 6,
		2, 6, 7,
		// Top
		0, 3, 4,
		0, 4, 7,
		// Bottom
		1, 2, 5,
		1, 5, 6
		*/
		
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
		
	};

	D3D11_BUFFER_DESC index_buffer_desc;
	index_buffer_desc = {
		sizeof(indices),
		D3D11_USAGE_IMMUTABLE,
		D3D11_BIND_INDEX_BUFFER,
		0,
		0,
		0
	};
	

	D3D11_SUBRESOURCE_DATA index_init_data;
	index_init_data.pSysMem = indices;

	HR(device_->CreateBuffer(&index_buffer_desc, &index_init_data, &index_buffer_));


}

void BoxApp::CreateFX() {
	
	UINT hlsl_flags = 0;
	UINT fx_flags = 0;
#if defined(DEBUG)||defined(_DEBUG)
	hlsl_flags |= D3D10_SHADER_DEBUG;
	hlsl_flags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	
	ID3DBlob *errors = NULL;
	// In new FX framework, compilation and creation can be finished in one single call
	HR(D3DX11CompileEffectFromFile(L"FX/color.fx", NULL, NULL, hlsl_flags, fx_flags, device_, &fx_, &errors));
	
	if (errors) {
		MessageBoxA(NULL, reinterpret_cast<char*>(errors->GetBufferPointer()), "Compiling FX error", 0);
		ReleaseCOM(errors);
	}
	
	fx_tech_ = fx_->GetTechniqueByName("ColorTech");
	fx_world_view_proj_ = fx_->GetVariableByName("g_world_view_proj")->AsMatrix();
	fx_time_ = fx_->GetVariableByName("g_time")->AsScalar();
	
	/*
	std::ifstream fin("FX/color.fxo", std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader;
	compiledShader.resize(size);

	fin.read(&compiledShader[0], size);
	fin.close();

	HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size,
		0, device_, &fx_));

	fx_tech_ = fx_->GetTechniqueByName("ColorTech");
	fx_world_view_proj_ = fx_->GetVariableByName("g_world_view_proj")->AsMatrix();
	*/
	
}

void BoxApp::CreateInputLayout() {
	/*
	D3D11_INPUT_ELEMENT_DESC vertex_element_desc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	*/

	//////////////////////////////////////////////////////////
	// Exercise 2: use two vertex buffers and two input slots.
	D3D11_INPUT_ELEMENT_DESC vertex_element_desc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	//////////////////////////////////////////////////////////

	D3DX11_PASS_DESC pass_desc;
	fx_tech_->GetPassByIndex(0)->GetDesc(&pass_desc);
	HR(device_->CreateInputLayout(vertex_element_desc, 2, pass_desc.pIAInputSignature, pass_desc.IAInputSignatureSize,
		&input_layout_));
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showcmd) {
#if defined(DEBUG)||defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	BoxApp app(hInstance);

	if (!app.Init())
		return 0;
	return app.Run();

}