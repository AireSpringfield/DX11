#include"d3dapp.h"
#include<fstream>
#include<vector>
#include"d3dx11effect.h"
#include"geometrygenerator.h"

using namespace DirectX;


struct Vertex {
	XMFLOAT3 pos;
	XMFLOAT4 color;
};

class SkullApp :public D3DApp {
public:
	SkullApp(HINSTANCE h);
	~SkullApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);
	void OnMouseWheel(WPARAM wParam, LPARAM lParam);

private:
	void CreateBuffers();
	void CreateFX();
	void CreateInputLayout();
private:
	ID3D11Buffer *vertex_buffer_ = NULL;
	ID3D11Buffer *index_buffer_ = NULL;

	ID3DX11Effect *fx_ = NULL;
	ID3DX11EffectTechnique *fx_tech_ = NULL;
	ID3DX11EffectMatrixVariable *fx_world_view_proj_ = NULL;

	ID3D11RasterizerState *rs_wireframe_ = NULL;
	ID3D11InputLayout *input_layout_ = NULL;

	XMFLOAT4X4 world_ = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 view_ = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 proj_ = MathHelper::XMFloat4x4Identity();

	float fov_ = 0.25*XM_PI;
	float near_z_ = 1.0f;
	float far_z_ = 1000.0f;

	float theta_ = 1.5*XM_PI;
	float phi_ = XM_PI / 6.0f;
	float radius_ = 15.0f; // radius_ == 0 will lead to error in XMMatrixLookAtLH

	UINT skull_index_cnt_;

	POINT last_mouse_pos_ = { 0, 0 };

};

SkullApp::SkullApp(HINSTANCE hInstance) : D3DApp(hInstance) {
	main_wnd_caption_ = L"Skull Demo";
}


SkullApp::~SkullApp() {
	ReleaseCOM(vertex_buffer_);
	ReleaseCOM(index_buffer_);
	ReleaseCOM(fx_);
	ReleaseCOM(input_layout_);
}

void SkullApp::OnResize() {
	D3DApp::OnResize();

	// Update projection matrix
	XMMATRIX proj = XMMatrixPerspectiveFovLH(fov_, AspectRatio(), near_z_, far_z_);
	XMStoreFloat4x4(&proj_, proj);
}

void SkullApp::UpdateScene(float dt) {
	float x = radius_*sinf(phi_)*cosf(theta_);
	float z = radius_*sinf(phi_)*sinf(theta_);
	float y = radius_*cosf(phi_);

	XMVECTOR eye = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR focus = XMVectorZero();

	XMMATRIX view = XMMatrixLookAtLH(eye, focus, up);
	XMStoreFloat4x4(&view_, view);

}

void SkullApp::DrawScene() {

	immediate_context_->ClearRenderTargetView(render_target_view_, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	immediate_context_->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	/************************************************************************************************/
	// Set input assemble, including input layout, primitive topology, vertex buffer, index buffers
	/************************************************************************************************/
	immediate_context_->IASetInputLayout(input_layout_);
	immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	immediate_context_->RSSetState(rs_wireframe_);

	UINT strides[1] = { sizeof(Vertex) };
	UINT offsets[1] = { 0 };
	immediate_context_->IASetVertexBuffers(0, 1, &vertex_buffer_, strides, offsets);
	immediate_context_->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);

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

		immediate_context_->DrawIndexed(skull_index_cnt_, 0, 0);
	}
	/********************************/
	// Present the swap chain
	/*******************************/
	HR(swap_chain_->Present(0, 0));
}

void SkullApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	last_mouse_pos_.x = x;
	last_mouse_pos_.y = y;

	SetCapture(main_wnd_);
}

void SkullApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void SkullApp::OnMouseMove(WPARAM btnState, int x, int y)
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
		// for we have deifsed up vector as (0,1,0). Also, when phi_ reaches 0 and take negative
		// values, up/down and left/right will be flipped.
		phi_ = MathHelper::Clamp(phi_, 0.01f, XM_PI / 2.0f);
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

void SkullApp::OnMouseWheel(WPARAM wParam, LPARAM lParam) {
	short delta = GET_WHEEL_DELTA_WPARAM(wParam);
	int inc = -delta / WHEEL_DELTA;
	radius_ += inc*5.0f;
	radius_ = MathHelper::Clamp(radius_, 0.5f, 100.0f);
}

bool SkullApp::Init() {
	if (!D3DApp::Init())
		return false;

	CreateBuffers();
	CreateFX();
	CreateInputLayout();

	D3D11_RASTERIZER_DESC wireframe_desc;
	wireframe_desc.FillMode = D3D11_FILL_WIREFRAME;
	wireframe_desc.CullMode = D3D11_CULL_BACK;
	wireframe_desc.FrontCounterClockwise = false;
	wireframe_desc.DepthClipEnable = true;
	HR(device_->CreateRasterizerState(&wireframe_desc, &rs_wireframe_))

	return true;
}



void SkullApp::CreateBuffers() {
	std::ifstream ifs("models/skull.txt");

	if (!ifs)
	{
		MessageBox(0, L"models/skull.txt not found.", 0, 0);
		return;
	}

	UINT vertex_cnt = 0;
	UINT tri_cnt = 0;
	std::string ignore;

	ifs >> ignore >> vertex_cnt;
	ifs >> ignore >> tri_cnt;
	ifs >> ignore; // "VertexList"
	ifs	>> ignore; // "(pos,"
	ifs >> ignore; // "normal)"
	ifs	>> ignore; // "{"

	float nx, ny, nz;
	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

	std::vector<Vertex> vertices(vertex_cnt);
	for (UINT i = 0; i < vertex_cnt; ++i)
	{
		ifs >> vertices[i].pos.x >> vertices[i].pos.y >> vertices[i].pos.z;

		vertices[i].color = black;

		// Normal not used in this demo.
		ifs >> nx >> ny >> nz;
	}

	ifs >> ignore;
	ifs >> ignore;
	ifs >> ignore;

	skull_index_cnt_ = 3 * tri_cnt;
	std::vector<UINT> indices(skull_index_cnt_);
	for (UINT i = 0; i < tri_cnt; ++i)
	{
		ifs >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}

	ifs.close();

	D3D11_BUFFER_DESC vertex_buffer_desc;
	vertex_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
	vertex_buffer_desc.ByteWidth = sizeof(Vertex) * vertex_cnt;
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_buffer_desc.CPUAccessFlags = 0;
	vertex_buffer_desc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vertex_init_data;
	vertex_init_data.pSysMem = &vertices[0];
	HR(device_->CreateBuffer(&vertex_buffer_desc, &vertex_init_data, &vertex_buffer_));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC index_buffer_desc;
	index_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
	index_buffer_desc.ByteWidth = sizeof(UINT) * skull_index_cnt_;
	index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	index_buffer_desc.CPUAccessFlags = 0;
	index_buffer_desc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA index_init_data;
	index_init_data.pSysMem = &indices[0];
	HR(device_->CreateBuffer(&index_buffer_desc, &index_init_data, &index_buffer_));
}

void SkullApp::CreateFX() {
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

void SkullApp::CreateInputLayout() {
	D3D11_INPUT_ELEMENT_DESC vertex_element_desc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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
	SkullApp app(hInstance);

	if (!app.Init())
		return 0;
	return app.Run();

}