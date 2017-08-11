//***************************************************************************************
// d3dApp.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include"d3dapp.h"
#include<Windows.h>
#include<sstream>
#include<vector>
#include<memory>
#include<iostream>
#include<cassert>
using namespace std;



namespace
{
	// This is just used to forward Windows messages from a global window
	// procedure to our member function window procedure because we cannot
	// assign a member function to WNDCLASS::lpfnWndProc.
	// Using a anonymous namespace, d3dapp us only available with in this file.
	// See https://www.gamedev.net/forums/topic/387255-wndclassexlpfnwndproc--member-function/.
	// You can think member functions have an extra parameter "this".
	// It is common to declare the function as static member.
	// However, static members can't be virtual
	//  Curiously Recursive Template Pattern (CRTP)?


	D3DApp* g_app = 0;
}


LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	return g_app->MsgProc(hwnd, msg, wParam, lParam);
}

D3DApp::D3DApp(HINSTANCE hInstance)
	: instance_(hInstance),
	main_wnd_(NULL),
	main_wnd_caption_(L"D3D11 Application"),
	driver_type_(D3D_DRIVER_TYPE_HARDWARE),
	client_width_(800),
	client_height_(600),
	enable_msaa4x_(false),
	is_paused_(false),
	is_minimized_(false),
	is_maximized_(false),
	is_resizing_(false),
	quality_msaa4x_(0),
	device_(NULL),
	immediate_context_(NULL),
	swap_chain_(NULL),
	depth_stencil_buffer_(NULL),
	render_target_view_(NULL),
	depth_stencil_view_(NULL)
{
	ZeroMemory(&viewport_, sizeof(D3D11_VIEWPORT));

	// Get a pointer to the application object so we can forward 
	// Windows messages to the object's window procedure through
	// the global window procedure.
	g_app = this;
}

D3DApp::~D3DApp()
{
	ReleaseCOM(render_target_view_);
	ReleaseCOM(depth_stencil_buffer_);
	ReleaseCOM(depth_stencil_view_);
	ReleaseCOM(swap_chain_);
	

	// Restore all default settings.
	if(immediate_context_)
		immediate_context_->ClearState();

	ReleaseCOM(immediate_context_);
	ReleaseCOM(device_);
}

void D3DApp::DrawCoordAxis() {

}


int D3DApp::Run()
{
	MSG msg = {0};
 
	timer_.Reset();

	while(msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if(PeekMessage( &msg, 0, 0, 0, PM_REMOVE ))
		{
            TranslateMessage( &msg );
            DispatchMessage( &msg );
		}
		// Otherwise, do animation/game stuff.
		else
        {	
			timer_.Tick();

			if( !is_paused_ )
			{
				CalculateFrameStats();
				UpdateScene(timer_.DeltaTime());	
				DrawScene();
			}
			else
			{
				// Suspend the process for 100 ms, so that a paused
				// application won't hog unnecessary CPU cycles
				Sleep(100);
			}
        }
    }

	return (int)msg.wParam;
}

bool D3DApp::Init()
{
	if(!InitMainWindow())
		return false;

	if(!InitDirect3D())
		return false;

	return true;
}
 

void D3DApp::OnResize()
{
	assert(device_);
	assert(immediate_context_);
	assert(swap_chain_);

	// Release the old views, as they hold references to the buffers we
	// will be destroying. 

	ReleaseCOM(render_target_view_);
	ReleaseCOM(depth_stencil_view_);
	// Also release the old depth/stencil buffer.
	ReleaseCOM(depth_stencil_buffer_);

	// Resize the swap chain 
	HR(swap_chain_->ResizeBuffers(1, client_width_, client_height_, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	// And recreate the render target view.
	ID3D11Texture2D* back_buffer;
	HR(swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&back_buffer)));
	HR(device_->CreateRenderTargetView(back_buffer, 0, &render_target_view_));
	ReleaseCOM(back_buffer);

	// Create the depth/stencil buffer and view.

	D3D11_TEXTURE2D_DESC depth_stencil_desc;
	
	depth_stencil_desc.Width     = client_width_;
	depth_stencil_desc.Height    = client_height_;
	depth_stencil_desc.MipLevels = 1;
	depth_stencil_desc.ArraySize = 1;
	depth_stencil_desc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// Use 4X MSAA? --must match swap chain MSAA values.
	if(enable_msaa4x_)
	{
		depth_stencil_desc.SampleDesc.Count   = 4;
		depth_stencil_desc.SampleDesc.Quality = quality_msaa4x_ - 1;
	}
	// No MSAA
	else
	{
		depth_stencil_desc.SampleDesc.Count   = 1;
		depth_stencil_desc.SampleDesc.Quality = 0;
	}

	depth_stencil_desc.Usage          = D3D11_USAGE_DEFAULT;
	depth_stencil_desc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
	depth_stencil_desc.CPUAccessFlags = 0; 
	depth_stencil_desc.MiscFlags      = 0;

	HR(device_->CreateTexture2D(&depth_stencil_desc, 0, &depth_stencil_buffer_));
	HR(device_->CreateDepthStencilView(depth_stencil_buffer_, 0, &depth_stencil_view_));


	// Bind the render target view and depth/stencil view to the pipeline.

	immediate_context_->OMSetRenderTargets(1, &render_target_view_, depth_stencil_view_);
	

	// Set the viewport transform.

	viewport_.TopLeftX = 0.0f;
	viewport_.TopLeftY = 0.0f;
	viewport_.Width    = static_cast<float>(client_width_);
	viewport_.Height   = static_cast<float>(client_height_);
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;

	/*
	viewport_.TopLeftX = 100.0f;
	viewport_.TopLeftY = 100.0f;
	viewport_.Width = 400.0f;
	viewport_.Height = 200.0f;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;
	*/

	immediate_context_->RSSetViewports(1, &viewport_);

	
}
LRESULT D3DApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	// WM_ACTIVATE is sent when the window is activated or deactivated.  
	// We pause the game when the window is deactivated and unpause it 
	// when it becomes active.  
	case WM_ACTIVATE:
		if( LOWORD(wParam) == WA_INACTIVE ) // LOWORD: low-ordered byte of the word
		{
			is_paused_= true;
			timer_.Pause();
		}
		else
		{
			is_paused_= false;
			timer_.Continue();
		}
		return 0;

	// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
		client_width_  = LOWORD(lParam);
		client_height_ = HIWORD(lParam);
		if( device_ )
		{
			if( wParam == SIZE_MINIMIZED )
			{
				is_paused_ = true;
				is_minimized_= true;
				is_maximized_ = false;
			}
			else if( wParam == SIZE_MAXIMIZED )
			{
				is_paused_ = false;
				is_minimized_ = false;
				is_maximized_ = true;
				OnResize();
			}
			else if( wParam == SIZE_RESTORED )
			{
				
				// Restoring from minimized state?
				if( is_minimized_)
				{
					is_paused_ = false;
					is_minimized_ = false;
					OnResize();
				}

				// Restoring from maximized state?
				else if( is_maximized_)
				{
					is_paused_ = false;
					is_maximized_ = false;
					OnResize();
				}
				else if(is_resizing_)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					OnResize();
				}
			}
		}
		return 0;

	// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		is_paused_ = true;
		is_resizing_ = true;
		timer_.Pause();
		return 0;

	// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
	// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		is_paused_ = false;
		is_resizing_  = false;
		timer_.Continue();
		OnResize();
		return 0;
 
	// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	// The WM_MENUCHAR message is sent when a menu is active and the user presses 
	// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
        // Don't beep when we alt-enter.
        return MAKELRESULT(0, MNC_CLOSE);

	// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200; 
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, LOWORD(lParam), HIWORD(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, LOWORD(lParam), HIWORD(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_MOUSEWHEEL:
		OnMouseWheel(wParam, lParam);
		return 0;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}


bool D3DApp::InitMainWindow()
{
	WNDCLASS wc;
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = MainWndProc; 
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = instance_;
	wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = L"D3DWndClassName";

	if( !RegisterClass(&wc) )
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, client_width_, client_height_};
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width  = R.right - R.left;
	int height = R.bottom - R.top;

	main_wnd_ = CreateWindow(L"D3DWndClassName", main_wnd_caption_.c_str(), 
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, instance_, 0); 
	if( !main_wnd_ )
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(main_wnd_, SW_SHOW);
	UpdateWindow(main_wnd_);

	return true;
}


bool D3DApp::InitDirect3D()
{
	// Create the device and device context.

	UINT create_device_flags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL feature_level;
	HRESULT hr = D3D11CreateDevice(
		NULL,                 // default adapter, 0 adapter enumerated by EnumAdapters
		driver_type_,
		NULL,                 // no software device
		create_device_flags,
		NULL, 0,              // default feature level array
		D3D11_SDK_VERSION,
		&device_,
		&feature_level,
		&immediate_context_);

	if (FAILED(hr))
	{
		MessageBox(NULL, L"D3D11CreateDevice Failed.", 0, 0);
		return false;
	}

	if (feature_level < D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
		return false;
	}

	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render 
	// target formats, so we only need to check quality support.

	HR(device_->CheckMultisampleQualityLevels(
		DXGI_FORMAT_R8G8B8A8_UNORM, 4, &quality_msaa4x_));
	assert(quality_msaa4x_ > 0);

	// Fill out a DXGI_SWAP_CHAIN_DESC to describe our swap chain.

	DXGI_SWAP_CHAIN_DESC swap_chain_desc;
	swap_chain_desc.BufferDesc.Width = client_width_;
	swap_chain_desc.BufferDesc.Height = client_height_;
	swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
	swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Use 4X MSAA? 
	if (enable_msaa4x_)
	{
		swap_chain_desc.SampleDesc.Count = 4;
		swap_chain_desc.SampleDesc.Quality = quality_msaa4x_ - 1;
	}
	// No MSAA
	else
	{
		swap_chain_desc.SampleDesc.Count = 1;
		swap_chain_desc.SampleDesc.Quality = 0;
	}

	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount = 1;
	swap_chain_desc.OutputWindow = main_wnd_;
	swap_chain_desc.Windowed = true;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swap_chain_desc.Flags = 0;

	// To correctly create the swap chain, we must use the IDXGIFactory that was
	// used to create the device.  If we tried to use a different IDXGIFactory instance
	// (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
	// This function is being called with a device from a different IDXGIFactory."

	IDXGIDevice* dxgi_device = NULL;
	HR(device_->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgi_device));

	IDXGIAdapter* dxgi_adapter = NULL;
	HR(dxgi_device->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgi_adapter));

	IDXGIFactory* dxgi_factory = NULL;
	HR(dxgi_adapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgi_factory));

	HR(dxgi_factory->CreateSwapChain(device_, &swap_chain_desc, &swap_chain_));

	// Disable Alt-Enter full screen mode.
	//dxgi_factory->MakeWindowAssociation(main_wnd_, DXGI_MWA_NO_WINDOW_CHANGES);
	dxgi_factory->MakeWindowAssociation(main_wnd_, DXGI_MWA_NO_ALT_ENTER);



	/*
	// Enumerate default adapter's outputs
	UINT num_modes;
	//DXGI_MODE_DESC desc; // Caution! That may lead to memory access violation with oss (Error code: 0xC0000005)
	// desc should be an array
	DXGI_MODE_DESC *desc = nullptr;
	IDXGIOutput *output;
	wostringstream oss;

	for (UINT i = 0;dxgi_adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND;++i) {

		oss << L"Output number: " << i << L'\n'
			<< L"*******************************\n";

		output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_SCALING, &num_modes, NULL);
		oss << L"Number of modes supported: " << num_modes << L'\n';

		desc = new DXGI_MODE_DESC[num_modes];
		HR(output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_SCALING, &num_modes, desc));

		for (UINT j = 0; j < num_modes;++j) {
			oss << L"Width = " << desc[j].Width 
				<< L"  Height = " << desc[j].Height
				<< L"  Refresh = " << desc[j].RefreshRate.Numerator << L'/' << desc[j].RefreshRate.Denominator
				<< L'\n';
		}
	}
	delete[] desc;
	OutputDebugString(oss.str().c_str());
		*/

	/*
	// Enumerate all the adapters (video cards).
	vector<IDXGIAdapter*> adapters;
	IDXGIAdapter* adapter;
	for (UINT i = 0;dxgi_factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND;++i)
		adapters.push_back(adapter);
	// Check interface support.
	// Note: IDXGIAdapter::CheckInterfaceSupport no longer applies to D3D11

	for (auto &r : adapters) {
		LARGE_INTEGER umd_version;
		HR(r->CheckInterfaceSupport(__uuidof(ID3D11Device), &umd_version));
		cout << umd_version.QuadPart << endl;
		ReleaseCOM(r);
	}
	*/
	ReleaseCOM(dxgi_device);
	ReleaseCOM(dxgi_adapter);
	ReleaseCOM(dxgi_factory);

	// The remaining steps that need to be carried out for d3d creation
	// also need to be executed every time the window is resized.  So
	// just call the OnResize method here to avoid code duplication.
	
	OnResize();

	return true;
}

void D3DApp::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	static int frame_cnt = 0;
	static float time_elapsed = 0.0f;

	frame_cnt++;

	// Compute averages over one second period.
	if( (timer_.TotalTime() - time_elapsed) >= 1.0f )
	{
		float fps = (float)frame_cnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		wostringstream outs;   
		outs.precision(6);
		outs << main_wnd_caption_ << L"    "
			 << L"FPS: " << fps << L"    " 
			 << L"Frame Time: " << mspf << L" (ms)";
		SetWindowText(main_wnd_, outs.str().c_str());
		
		// Reset for next average.
		frame_cnt = 0;
		time_elapsed += 1.0f;
	}
}
