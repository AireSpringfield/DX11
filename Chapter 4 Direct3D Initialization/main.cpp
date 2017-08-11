#include"d3dapp.h"	



class InitD3DApp :public D3DApp {
public:
	InitD3DApp(HINSTANCE hInstance) :D3DApp(hInstance) {}

public:
	// Override virtual pure functions
	void UpdateScene(float dt) {

	}

	void DrawScene() {
		assert(immediate_context_);
		assert(swap_chain_);

		immediate_context_->ClearRenderTargetView(render_target_view_, Colors::Blue);
		immediate_context_->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		HR(swap_chain_->Present(0, 0));
	}

};



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showcmd) {
	InitD3DApp app(hInstance);
	app.Init();
	return app.Run();
}