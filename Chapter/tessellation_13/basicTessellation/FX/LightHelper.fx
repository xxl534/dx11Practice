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