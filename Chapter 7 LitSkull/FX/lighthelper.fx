//***************************************************************************************
// LightHelper.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Structures and functions for lighting calculations.
//***************************************************************************************

struct DirectionalLight
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float3 direction;
	float pad;
};

struct PointLight
{ 
	float4 ambient;
	float4 diffuse;
	float4 specular;

	float3 position;
	float range;

	float3 att;
	float pad;
};

struct SpotLight
{
	float4 ambient;
	float4 diffuse;
	float4 specular;

	float3 position;
	float range;

	float3 direction; // Central direction.
	float spot; // Power attenuation.

	float3 att;
	float pad;
};

struct Material
{
	float4 ambient;
	float4 diffuse;
	float4 specular; // w = SpecPower
	float4 reflect;
};

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a directional light.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
void ComputeDirectionalLight(Material material, DirectionalLight light, 
                             float3 normal, float3 to_eye,
					         out float4 ambient,
						     out float4 diffuse,
						     out float4 specular)
{
	// Initialize outputs.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The light vector aims opposite the direction the light rays travel.
	float3 l = -light.direction;

	// Add ambient term.
	ambient = material.ambient * light.ambient;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	
	float kd = dot(l, normal);

	// Flatten to avoid dynamic branching.
	[flatten]
	if( kd > 0.0f )
	{
        float3 r = reflect(-l, normal);
		float ks = pow(max(dot(r, to_eye), 0.0f), material.specular.w);
					
        diffuse = kd * material.diffuse * light.diffuse;
		specular = ks * material.specular * light.specular;
	}
}

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a point light.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
void ComputePointLight(Material material, PointLight light, float3 pos, float3 normal, float3 to_eye,
				   out float4 ambient, out float4 diffuse, out float4 specular)
{
	// Initialize outputs.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The vector from the surface to the light.
	float3 l= light.position - pos;
		
	// The distance from surface to light.
	float d = length(l);
	
	// Range test.
	if( d > light.range )
		return;
		
	// Normalize the light vector.
	l /= d; 
	
	// Ambient term.
	ambient = material.ambient * light.ambient;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.

	float kd = dot(l, normal);

	// Flatten to avoid dynamic branching.
	[flatten]
	if( kd > 0.0f )
	// kd must be positive, assuring surface is lit from the front.
	{
        float3 r = reflect(-l, normal);
		float ks= pow(max(dot(r, to_eye), 0.0f), material.specular.w);
					
		diffuse = kd * material.diffuse * light.diffuse;
		specular = ks * material.specular * light.specular;
	}

	// Attenuate
	float att = 1.0f / dot(light.att, float3(1.0f, d, d*d));

	diffuse *= att;
	specular *= att;
}

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a spotlight.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
void ComputeSpotLight(Material material, SpotLight light, float3 pos, float3 normal, float3 to_eye,
				  out float4 ambient, out float4 diffuse, out float4 specular)
{
	// Initialize outputs.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The vector from the surface to the light.
	float3 l = light.position - pos;
		
	// The distance from surface to light.
	float d = length(l);
	
	// Range test.
	if( d > light.range )
		return;
		
	// Normalize the light vector.
	l /= d; 
	
	// Ambient term.
	ambient = material.ambient * light.ambient;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.

	float kd = dot(l, normal);

	// Flatten to avoid dynamic branching.
	[flatten]
	if( kd > 0.0f )
	{
        float3 r = reflect(-l, normal);
		float ks = pow(max(dot(r, to_eye), 0.0f), material.specular.w);
					
		diffuse = kd * material.diffuse * light.diffuse;
		specular    = ks * material.specular * light.specular;
	}
	
	/**************************************************************/
	// Scale by spotlight factor and attenuate.
	// This is for spot light.
	float spot = pow(max(dot(-l, light.direction), 0.0f), light.spot);
	/**************************************************************/

	// Scale by spotlight factor and attenuate.
	float att = spot / dot(light.att, float3(1.0f, d, d*d));

	ambient *= spot; // Ambient light is not affected by cone range of spot light.
	diffuse *= att;
	specular *= att;
}

 
 