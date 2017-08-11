cbuffer cbPerObject {
	float4x4 g_world_view_proj;
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
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}