//***************************************************************************************
// VecAddDemo.cpp by Frank Luna (C) 2011 All Rights Reserved.
//
// Demonstrates structured buffers and a compute shader that adds corresponding
// structured buffer components.  The result is copied to system memory and written
// to file (results.txt).
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

struct Data
{
	XMFLOAT3 v1;
	XMFLOAT2 v2;
};

class VecAddApp : public D3DApp
{
public:
	VecAddApp(HINSTANCE hInstance);
	~VecAddApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene(); 

	void DoComputeWork();

private:
	void BuildBuffersAndViews();
	
private:
	ID3D11Buffer* mOutputBuffer;
	ID3D11Buffer* mOutputDebugBuffer;

	ID3D11ShaderResourceView* mInputASRV;
	ID3D11ShaderResourceView* mInputBSRV;
	ID3D11UnorderedAccessView* mOutputUAV;

	UINT mNumElements;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	VecAddApp theApp(hInstance);
	
	if( !theApp.Init() )
		return 0;
	
	theApp.DoComputeWork();
	

	return 0;
}

VecAddApp::VecAddApp(HINSTANCE hInstance)
: D3DApp(hInstance), mOutputBuffer(0), mOutputDebugBuffer(0),
	mInputASRV(0), mInputBSRV(0), mOutputUAV(0), mNumElements(32)
{
	main_wnd_caption_ = L"Compute Shader Vec Add Demo";
}

VecAddApp::~VecAddApp()
{
	immediate_context_->ClearState();

	ReleaseCOM(mOutputBuffer);
	ReleaseCOM(mOutputDebugBuffer);

	ReleaseCOM(mInputASRV);
	ReleaseCOM(mInputBSRV);
	ReleaseCOM(mOutputUAV);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool VecAddApp::Init()
{
	if(!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(device_);
	InputLayouts::InitAll(device_);
	RenderStates::InitAll(device_);

	BuildBuffersAndViews();

	return true;
}

void VecAddApp::OnResize()
{
	D3DApp::OnResize();
}

void VecAddApp::UpdateScene(float dt)
{
	
}

void VecAddApp::DrawScene()
{
	immediate_context_->ClearRenderTargetView(render_target_view_, reinterpret_cast<const float*>(&Colors::Silver));
	immediate_context_->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	HR(swap_chain_->Present(0, 0));
}

void VecAddApp::DoComputeWork()
{
	D3DX11_TECHNIQUE_DESC techDesc;

	Effects::VecAddFX->SetInputA(mInputASRV);
	Effects::VecAddFX->SetInputB(mInputBSRV);
	Effects::VecAddFX->SetOutput(mOutputUAV);

	Effects::VecAddFX->VecAddTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = Effects::VecAddFX->VecAddTech->GetPassByIndex(p);
		pass->Apply(0, immediate_context_);

		immediate_context_->Dispatch(1, 1, 1);
	}

	// Unbind the input textures from the CS for good housekeeping.
	ID3D11ShaderResourceView* nullSRV[1] = { 0 };
	immediate_context_->CSSetShaderResources( 0, 1, nullSRV );

	// Unbind output from compute shader (we are going to use this output as an input in the next pass, 
	// and a resource cannot be both an output and input at the same time.
	ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
	immediate_context_->CSSetUnorderedAccessViews( 0, 1, nullUAV, 0 );

	// Disable compute shader.
	immediate_context_->CSSetShader(0, 0, 0);

	std::ofstream fout("results.txt");

	// Copy the output buffer to system memory.
	immediate_context_->CopyResource(mOutputDebugBuffer, mOutputBuffer);

	// Map the data for reading.
	D3D11_MAPPED_SUBRESOURCE mappedData; 
    immediate_context_->Map(mOutputDebugBuffer, 0, D3D11_MAP_READ, 0, &mappedData);

	Data* dataView = reinterpret_cast<Data*>(mappedData.pData);

	for(int i = 0; i < mNumElements; ++i)
	{
		fout << "(" << dataView[i].v1.x << ", " << dataView[i].v1.y << ", " << dataView[i].v1.z <<
			", " << dataView[i].v2.x << ", " << dataView[i].v2.y << ")" << std::endl;
	}

    immediate_context_->Unmap(mOutputDebugBuffer, 0);

	fout.close();

	MessageBox(main_wnd_, L"Vector adding finished!", 0, 0);
}

void VecAddApp::BuildBuffersAndViews()
{
	std::vector<Data> dataA(mNumElements);
	std::vector<Data> dataB(mNumElements);
	for(int i = 0; i < mNumElements; ++i)
	{
		dataA[i].v1 = XMFLOAT3(i, i, i);
		dataA[i].v2 = XMFLOAT2(i, 0);

		dataB[i].v1 = XMFLOAT3(-i, i, 0.0f);
		dataB[i].v2 = XMFLOAT2(0, -i);
	}

	// Create a buffer to be bound as a shader input (D3D11_BIND_SHADER_RESOURCE).
	D3D11_BUFFER_DESC inputDesc;
    inputDesc.Usage = D3D11_USAGE_DEFAULT;
    inputDesc.ByteWidth = sizeof(Data) * mNumElements;
    inputDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    inputDesc.CPUAccessFlags = 0;
	inputDesc.StructureByteStride = sizeof(Data);
    inputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    D3D11_SUBRESOURCE_DATA vinitDataA;
    vinitDataA.pSysMem = &dataA[0];

	ID3D11Buffer* bufferA = 0;
    HR(device_->CreateBuffer(&inputDesc, &vinitDataA, &bufferA));

	D3D11_SUBRESOURCE_DATA vinitDataB;
    vinitDataB.pSysMem = &dataB[0];

	ID3D11Buffer* bufferB = 0;
    HR(device_->CreateBuffer(&inputDesc, &vinitDataB, &bufferB));

	// Create a read-write buffer the compute shader can write to (D3D11_BIND_UNORDERED_ACCESS).
	D3D11_BUFFER_DESC outputDesc;
    outputDesc.Usage = D3D11_USAGE_DEFAULT;
    outputDesc.ByteWidth = sizeof(Data) * mNumElements;
    outputDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    outputDesc.CPUAccessFlags = 0;
	outputDesc.StructureByteStride = sizeof(Data);
    outputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

    HR(device_->CreateBuffer(&outputDesc, 0, &mOutputBuffer));

	// Create a system memory version of the buffer to read the results back from.
	outputDesc.Usage = D3D11_USAGE_STAGING;
	outputDesc.BindFlags = 0;
	outputDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	HR(device_->CreateBuffer(&outputDesc, 0, &mOutputDebugBuffer));


	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.BufferEx.FirstElement = 0;
	srvDesc.BufferEx.Flags = 0;
	srvDesc.BufferEx.NumElements = mNumElements;

    device_->CreateShaderResourceView(bufferA, &srvDesc, &mInputASRV);
	device_->CreateShaderResourceView(bufferB, &srvDesc, &mInputBSRV);


	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = 0;
	uavDesc.Buffer.NumElements = mNumElements;

	device_->CreateUnorderedAccessView(mOutputBuffer, &uavDesc, &mOutputUAV);


	// Views hold references to buffers, so we can release these.
	ReleaseCOM(bufferA);
	ReleaseCOM(bufferB);
}
