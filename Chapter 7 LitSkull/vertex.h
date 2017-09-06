#ifndef VERTEX_H
#define VERTEX_H

#include"d3dutility.h"
#include<DirectXMath.h>
using namespace DirectX;

namespace Vertex {
	struct PosNormal {
		XMFLOAT3 pos;
		XMFLOAT3 normal;
	};
}

class InputLayoutDesc {
public:
	static const D3D11_INPUT_ELEMENT_DESC desc[2];
};



class InputLayouts {
public:
	static void InitAll(ID3D11Device *device);
	static void DestroyAll();
	
	static ID3D11InputLayout *posNormal;
};

#endif
