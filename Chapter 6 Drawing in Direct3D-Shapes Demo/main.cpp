#include"d3dapp.h"
#include"d3dx11effect.h"
#include"geometrygenerator.h"
#include<fstream>
#include<vector>

using namespace DirectX;




struct Vertex {
	XMFLOAT3 pos;
	XMFLOAT4 color;
};

class ShapesApp :public D3DApp {
public:
	ShapesApp(HINSTANCE h);
	~ShapesApp();

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

	ID3D11InputLayout *input_layout_ = NULL;
	ID3D11RasterizerState *rs_wireframe_ = NULL;

	
	XMFLOAT4X4 view_ = MathHelper::XMFloat4x4Identity();
	XMFLOAT4X4 proj_ = MathHelper::XMFloat4x4Identity();

	// World transformations are possessed by different objects
	XMFLOAT4X4 world_box_;
	XMFLOAT4X4 world_grid_;
	XMFLOAT4X4 world_sphere_[10];
	XMFLOAT4X4 world_cylinder_[10];
	XMFLOAT4X4 world_center_sphere_;

	float fov_ = 0.25*XM_PI;
	float near_z_ = 1.0f;
	float far_z_ = 1000.0f;

	float theta_ = 1.5*XM_PI;
	float phi_ = 0.25*XM_PI;
	float radius_ = 2.0f;

	POINT last_mouse_pos_ = { 0, 0 };

	/******************************************/
	// Variables for facilitating use of buffer
	/******************************************/

	// Vertex offsets
	UINT box_vertex_offset_ = 0;
	UINT grid_vertex_offset_ = 0;
	UINT sphere_vertex_offset_ = 0;
	UINT cylinder_vertex_offset_ = 0;

	// Number of indices for each kind of shapes
	UINT box_index_cnt_ = 0;
	UINT grid_index_cnt_ = 0;
	UINT sphere_index_cnt_ = 0;
	UINT cylinder_index_cnt = 0;

	// Offset indices to draw a shape at a time
	UINT box_index_offset_ = 0;
	UINT grid_index_offset_ = 0;
	UINT sphere_index_offset_ = 0;
	UINT cylinder_index_offset_ = 0;
};

ShapesApp::ShapesApp(HINSTANCE hInstance) : D3DApp(hInstance) {
	main_wnd_caption_ = L"Shapes Demo";
	// Transformations from local to world
	// Box
	XMMATRIX scale_box = XMMatrixScaling(2.0f, 1.0f, 2.0f);
	XMMATRIX trans_box = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	XMStoreFloat4x4(&world_box_, XMMatrixMultiply(scale_box, trans_box));
	// Grid
	world_grid_ = MathHelper::XMFloat4x4Identity();
	// Center sphere
	XMMATRIX scale_center_sphere = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	XMMATRIX trans_center_sphere = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
	XMStoreFloat4x4(&world_center_sphere_, XMMatrixMultiply(scale_center_sphere, trans_center_sphere));

	for (UINT i = 0; i < 5; ++i)
	{
		XMStoreFloat4x4(&world_cylinder_[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i*5.0f));
		XMStoreFloat4x4(&world_cylinder_[i * 2 + 1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i*5.0f));

		XMStoreFloat4x4(&world_sphere_[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i*5.0f));
		XMStoreFloat4x4(&world_sphere_[i * 2 + 1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i*5.0f));
	}

}


ShapesApp::~ShapesApp() {
	ReleaseCOM(vertex_buffer_);
	ReleaseCOM(index_buffer_);
	ReleaseCOM(fx_);
	ReleaseCOM(input_layout_);
	ReleaseCOM(rs_wireframe_)

}

void ShapesApp::OnResize() {
	D3DApp::OnResize();

	// Update projection matrix
	XMMATRIX proj = XMMatrixPerspectiveFovLH(fov_, AspectRatio(), near_z_, far_z_);
	XMStoreFloat4x4(&proj_, proj);
}

void ShapesApp::UpdateScene(float dt) {
	float x = radius_*sinf(phi_)*cosf(theta_);
	float z = radius_*sinf(phi_)*sinf(theta_);
	float y = radius_*cosf(phi_);

	XMVECTOR eye = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR focus = XMVectorZero();

	XMMATRIX view = XMMatrixLookAtLH(eye, focus, up);
	XMStoreFloat4x4(&view_, view);

}

void ShapesApp::DrawScene() {

	immediate_context_->ClearRenderTargetView(render_target_view_, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	immediate_context_->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	/************************************************************************************************/
	// Set input assemble, including input layout, primitive topology, vertex buffer, index buffers
	/************************************************************************************************/
	immediate_context_->IASetInputLayout(input_layout_);
	immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	/***********************************************/
	// Set rasterizer state we've created
	/***********************************************/
	immediate_context_->RSSetState(rs_wireframe_);

	UINT strides[1] = { sizeof(Vertex) };
	UINT offsets[1] = { 0 };
	immediate_context_->IASetVertexBuffers(0, 1, &vertex_buffer_, strides, offsets);
	immediate_context_->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);


	XMMATRIX view = XMLoadFloat4x4(&view_);
	XMMATRIX proj = XMLoadFloat4x4(&proj_);
	XMMATRIX view_proj = view*proj;

	D3DX11_TECHNIQUE_DESC tech_desc;
	fx_tech_->GetDesc(&tech_desc);
	for (UINT i = 0; i < tech_desc.Passes; ++i) {
		// Draw the grid.
		
		XMMATRIX world = XMLoadFloat4x4(&world_grid_);
		
		fx_world_view_proj_->SetMatrix(reinterpret_cast<float*>(&(world*view_proj)));
		fx_tech_->GetPassByIndex(i)->Apply(0, immediate_context_);
		immediate_context_->DrawIndexed(grid_index_cnt_, grid_index_offset_, grid_vertex_offset_);
		

		
		// Draw the box.
		world = XMLoadFloat4x4(&world_box_);
		fx_world_view_proj_->SetMatrix(reinterpret_cast<float*>(&(world*view_proj)));
		fx_tech_->GetPassByIndex(i)->Apply(0, immediate_context_);
		immediate_context_->DrawIndexed(box_index_cnt_, box_index_offset_, box_vertex_offset_);

		// Draw center sphere.
		world = XMLoadFloat4x4(&world_center_sphere_);
		fx_world_view_proj_->SetMatrix(reinterpret_cast<float*>(&(world*view_proj)));
		fx_tech_->GetPassByIndex(i)->Apply(0, immediate_context_);
		immediate_context_->DrawIndexed(sphere_index_cnt_, sphere_index_offset_, sphere_vertex_offset_);

		// Draw the cylinders.
		for (int j = 0; j< 10; ++j)
		{
			world = XMLoadFloat4x4(&world_cylinder_[j]);
			fx_world_view_proj_->SetMatrix(reinterpret_cast<float*>(&(world*view_proj)));
			fx_tech_->GetPassByIndex(i)->Apply(0, immediate_context_);
			immediate_context_->DrawIndexed(cylinder_index_cnt, cylinder_index_offset_, cylinder_vertex_offset_);
		}

		// Draw the spheres.
		for (int j = 0; j < 10; ++j)
		{
			world = XMLoadFloat4x4(&world_sphere_[j]);
			fx_world_view_proj_->SetMatrix(reinterpret_cast<float*>(&(world*view_proj)));
			fx_tech_->GetPassByIndex(i)->Apply(0, immediate_context_);
			immediate_context_->DrawIndexed(sphere_index_cnt_, sphere_index_offset_, sphere_vertex_offset_);
		}
		
	}
	/********************************/
	// Present the swap chain
	/*******************************/
	HR(swap_chain_->Present(0, 0));
}

void ShapesApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	last_mouse_pos_.x = x;
	last_mouse_pos_.y = y;

	SetCapture(main_wnd_);
}

void ShapesApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture(); 
}

void ShapesApp::OnMouseMove(WPARAM btnState, int x, int y)
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
		phi_ = MathHelper::Clamp(phi_, 0.001f, XM_PI / 2.0f);
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

void ShapesApp::OnMouseWheel(WPARAM wParam, LPARAM lParam) {
	short delta = GET_WHEEL_DELTA_WPARAM(wParam);
	int inc = -delta / WHEEL_DELTA;
	radius_ += inc*2.0f;
	radius_ = MathHelper::Clamp(radius_, 3.0f, 500.0f);
}

bool ShapesApp::Init() {
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
	HR(device_->CreateRasterizerState(&wireframe_desc, &rs_wireframe_));

	return true;
}



void ShapesApp::CreateBuffers() {
	GeometryGenerator::MeshData cylinder, sphere, box, grid;
	GeometryGenerator geogen;

	geogen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geogen.CreateGrid(20.0f, 30.0f, 40, 60, grid);
	geogen.CreateGeosphere(0.5f, 5, sphere);
	geogen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, cylinder);

	// Vertex offset is the position of first vertex in the vertex buffer
	box_vertex_offset_ = 0;
	grid_vertex_offset_ = box_vertex_offset_ + box.vertices.size();
	sphere_vertex_offset_ = grid_vertex_offset_ + grid.vertices.size();
	cylinder_vertex_offset_ = sphere_vertex_offset_ + sphere.vertices.size();

	// Index count tells how many indices used to draw shapes
	box_index_cnt_ = box.indices.size();
	grid_index_cnt_ = grid.indices.size();
	sphere_index_cnt_ = sphere.indices.size();
	cylinder_index_cnt = cylinder.indices.size();

	// Index offset also gives information about each shape's index start point
	box_index_offset_ = 0;
	grid_index_offset_ = box_index_offset_ + box_index_cnt_;
	sphere_index_offset_ = grid_index_offset_ + grid_index_cnt_;
	cylinder_index_offset_ = sphere_index_offset_ + sphere_index_cnt_;

	UINT total_vertex_cnt = cylinder_vertex_offset_ + cylinder.vertices.size();
	UINT total_index_cnt = cylinder_index_offset_ + cylinder_index_cnt;

	std::vector<Vertex> vertices(total_vertex_cnt);
	
	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

	auto iter = vertices.begin();

	for (UINT i = 0; i < box.vertices.size(); ++i, ++iter) {
		iter->pos = box.vertices[i].position;
		iter->color = black;
	}

	for (UINT i = 0; i < grid.vertices.size(); ++i, ++iter) {
		iter->pos = grid.vertices[i].position;
		iter->color = black;
	}

	for (UINT i = 0; i < sphere.vertices.size(); ++i, ++iter) {
		iter->pos = sphere.vertices[i].position;
		iter->color = black;
	}

	for (UINT i = 0; i < cylinder.vertices.size(); ++i, ++iter) {
		iter->pos = cylinder.vertices[i].position;
		iter->color = black;
	}

	std::vector<UINT> indices;

	indices.insert(indices.end(), box.indices.begin(), box.indices.end());
	indices.insert(indices.end(), grid.indices.begin(), grid.indices.end());
	indices.insert(indices.end(), sphere.indices.begin(), sphere.indices.end());
	indices.insert(indices.end(), cylinder.indices.begin(), cylinder.indices.end());	


	D3D11_BUFFER_DESC vertex_buffer_desc = {
		sizeof(Vertex) * vertices.size(),
		D3D11_USAGE_IMMUTABLE,
		D3D11_BIND_VERTEX_BUFFER,
		0,
		0,
		0
	};
	D3D11_SUBRESOURCE_DATA vertex_init_data;
	vertex_init_data.pSysMem = &vertices[0];
	HR(device_->CreateBuffer(&vertex_buffer_desc, &vertex_init_data, &vertex_buffer_));


	D3D11_BUFFER_DESC index_buffer_desc;
	index_buffer_desc = {
		sizeof(UINT)*indices.size(),
		D3D11_USAGE_IMMUTABLE,
		D3D11_BIND_INDEX_BUFFER,
		0,
		0,
		0
	};
	D3D11_SUBRESOURCE_DATA index_init_data;
	index_init_data.pSysMem = &indices[0];
	HR(device_->CreateBuffer(&index_buffer_desc, &index_init_data, &index_buffer_));
}

void ShapesApp::CreateFX() {
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

void ShapesApp::CreateInputLayout() {
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
	ShapesApp app(hInstance);

	if (!app.Init())
		return 0;
	return app.Run();

}