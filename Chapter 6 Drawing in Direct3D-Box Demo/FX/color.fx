////////////////////////////////////////////////////////////////////////
// Exercise 6: distort the vertices with sine function in vertex shader
////////////////////////////////////////////////////////////////////////
// Exercise 8/9: Set rasterizer state in an Effect pass
////////////////////////////////////////////////////////////////////////


cbuffer cbPerObject {
	float4x4 g_world_view_proj;
    float g_time; 
};

RasterizerState WireframeRS
{
    FillMode = Wireframe;
    CullMode = Front;
    FrontCounterClockwise = false;
};


struct VertexIn {
	float3 pos_local : POSITION;
	float4 color : COLOR;
};

struct VertexOut {
	float4 pos_homo : SV_POSITION;
	float4 color : COLOR;
};

VertexOut VS(VertexIn vin) {
	VertexOut vout;
    //////////////////////////////////////////////////////////////////////
    vin.pos_local.xy += 0.5f * sin(3.0f * g_time);
    vin.pos_local.z += 0.4f * sin(2.0f * g_time);
    //////////////////////////////////////////////////////////////////////

	vout.pos_homo = mul(float4(vin.pos_local, 1.0f), g_world_view_proj);
	vout.color = vin.color;
	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	return pin.color;
}

technique11 ColorTech {
    pass P0{
        SetRasterizerState(WireframeRS);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}