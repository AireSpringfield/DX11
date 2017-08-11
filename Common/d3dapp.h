//***************************************************************************************
// d3dApp.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Simple Direct3D demo application class.  
// Make sure you link: d3d11.lib d3dx11d.lib D3DCompiler.lib D3DX11EffectsD.lib 
//                     dxerr.lib dxgi.lib dxguid.lib.
// Link d3dx11.lib and D3DX11Effects.lib for release mode builds instead
//   of d3dx11d.lib and D3DX11EffectsD.lib.
//***************************************************************************************

#ifndef D3DAPP_H
#define D3DAPP_H

#include "d3dutility.h"
#include "d3dtimer.h"
#include <string>

class D3DApp
{
public:
	D3DApp(HINSTANCE hInstance);
	virtual ~D3DApp();
	
	// Inline access methods.
	inline HINSTANCE AppInst() const {
		return instance_;
	}
	HWND  inline MainWnd() const {
		return main_wnd_;
	}
	float AspectRatio() const {
		return static_cast<float>(client_width_) / client_height_;
	};
	
	// Wrap the message loop.
	int Run();
 
	// Framework methods.  Derived client class overrides these methods to 
	// implement specific application requirements.

	virtual bool Init();
	virtual void OnResize(); 
	virtual void UpdateScene(float dt) = 0;
	virtual void DrawScene() = 0; 
	void DrawCoordAxis();

	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// Convenience overrides for handling mouse input.
	virtual void OnMouseDown(WPARAM btnState, int x, int y){ }
	virtual void OnMouseUp(WPARAM btnState, int x, int y)  { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y){ }
	virtual void OnMouseWheel(WPARAM wParam, LPARAM lParam){ }

protected:
	bool InitMainWindow();
	bool InitDirect3D();

	void CalculateFrameStats();

protected:

	HINSTANCE instance_;
	HWND      main_wnd_;
	bool      is_paused_;
	bool      is_minimized_;
	bool      is_maximized_;
	bool      is_resizing_;
	UINT      quality_msaa4x_;

	D3DTimer timer_;
	
	

	ID3D11Device* device_;
	ID3D11DeviceContext* immediate_context_;
	IDXGISwapChain* swap_chain_;
	ID3D11Texture2D* depth_stencil_buffer_;
	ID3D11RenderTargetView* render_target_view_;
	ID3D11DepthStencilView* depth_stencil_view_;
	D3D11_VIEWPORT viewport_;

	// Derived class should set these in derived constructor to customize starting values.
	std::wstring main_wnd_caption_;
	D3D_DRIVER_TYPE driver_type_;
	int client_width_;
	int client_height_;
	bool enable_msaa4x_;
};

#endif // D3DAPP_H