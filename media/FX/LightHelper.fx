struct DirectionalLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Direction;
	float pad;
};

struct PointLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	
	float3 Position;
	float 	Range;
	
	float3 Att;
	float pad;
};

struct SpotLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	
	float3 Position;
	float Range;
	
	float3 Direction;
	float Spot;
	
	float3 Att;
	float pad;
};

struct Material
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float4 Reflect;
};

void ComputeDirectionalLight(Material mat, DirectionalLight L, float3 normal, float3 toEye, out float4 ambient, out float4 diffuse, out float4 spec)
{
	ambient = float4( 0.f, 0.f, 0.f, 0.f );
	diffuse = float4( 0.f, 0.f, 0.f, 0.f );
	spec 	= float4( 0.f, 0.f, 0.f, 0.f );
	
	float3 lightVec = -L.Direction;
	
	ambient = mat.Ambient * L.Ambient;
	
	float diffuseFactor = dot( lightVec, normal );
	
	[flatten]
	if( diffuseFactor > 0.f )
	{
		float3 v = reflect(-lightVec,normal);
		float specFactor = pow( max(dot( v, toEye), 0.f ), mat.Specular.w );
		
		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec = specFactor * mat.Specular * L.Specular;
	}
}

void ComputePointLight(Material mat, PointLight L, float3 pos, float3 normal, float3 toEye, out float4 ambient, out float4 diffuse, out float4 spec)
{
	ambient = float4( 0.f, 0.f, 0.f, 0.f );
	diffuse = float4( 0.f, 0.f, 0.f, 0.f );
	spec 	= float4( 0.f, 0.f, 0.f, 0.f );
	
	float3 lightVec = L.Position - pos;
	float d = length(lightVec);
	if( d > L.Range )
	{
		return;
	}
	
	lightVec /= d;
	ambient = mat.Ambient * L.Ambient;
	
	float diffuseFactor = dot( lightVec, normal );
	[flatten]
	if( diffuseFactor > 0.f )
	{
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow( max( dot( v, toEye ), 0.f ), mat.Specular.w );
		
		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec = specFactor * mat.Specular * L.Specular;
	}
	
	float att = 1.0/ dot(L.Att, float3(1.f, d, d*d) );
	diffuse *= att;
	spec *= att;
}

void ComputeSpotLight(Material mat, SpotLight L, float3 pos, float3 normal, float3 toEye, out float4 ambient, out float4 diffuse, out float4 spec )
{
	ambient = float4( 0.f, 0.f, 0.f, 0.f );
	diffuse = float4( 0.f, 0.f, 0.f, 0.f );
	spec 	= float4( 0.f, 0.f, 0.f, 0.f );
	
	float3 lightVec = L.Position - pos;
	float d = length(lightVec);
	if( d > L.Range )
	{
		return;
	}
	
	lightVec /= d;
	ambient = mat.Ambient * L.Ambient;
	
	float diffuseFactor = dot( lightVec, normal );
	[flatten]
	if( diffuseFactor > 0.f )
	{
		float3 v = reflect( -lightVec, normal );
		float specFactor = pow(max(dot(v,toEye),0.f),mat.Specular.w);
		
		diffuse= diffuseFactor * mat.Diffuse * L.Diffuse;
		spec = specFactor * mat.Specular * L.Specular;
	}
	
	float spot = pow( max( dot( -lightVec, L.Direction ), 0.f ), L.Spot );
	
	float att = spot / dot( L.Att, float3( 1.f, d, d*d));
	ambient *= spot;
	diffuse *= att;
	spec *= att;
}

//---------------------------------------------------------------------------------------
// Transforms a normal map sample to world space.
//---------------------------------------------------------------------------------------
float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
	// Uncompress each component from [0,1] to [-1,1].
	float3 normalT = 2.0f*normalMapSample - 1.0f;

	// Build orthonormal basis.
	float3 N = unitNormalW;
	float3 T = normalize(tangentW - dot(tangentW, N)*N);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
	float3 bumpedNormalW = mul(normalT, TBN);

	return bumpedNormalW;
}

//---------------------------------------------------------------------------------------
// Performs shadowmap test to determine if a pixel is in shadow.
//---------------------------------------------------------------------------------------
static const float SMAP_SIZE = 2048.f;
static const float SMAP_DX = 1.f/SMAP_SIZE;

float CalcShadowFactor(SamplerComparisonState samShadow,
						Texture2D shadowMap,
						float4 	shadowPosH)
{
	shadowPosH.xyz /= shadowPosH.w;
	
	float depth = shadowPosH.z;
	
	const float d = SMAP_DX;
	
	float percentLit = 0.f;
	const float2 offsets[9] = 
	{
		float2(-d,-d),float2(0.f,-d),float2(d,-d),
		float2(-d,0.f),float2(0.f,0.f),float2(d,0.f),
		float2(-d,d),float2(0.f,d),float2(d,d)
	};
	
	[unroll]
	for(int i = 0; i < 9; ++i)
	{
		percentLit += shadowMap.SampleCmpLevelZero(samShadow,
			shadowPosH.xy + offsets[i], depth).r;
	}
	
	return percentLit /= 9.f;
}