$input v_texcoord0

#include <bgfx_shader.sh>
#include "shaderlib.sh"
#include "common.sh"

SAMPLER2D(s_albedo,  0);
SAMPLER2D(s_normal, 1);
SAMPLER2D(s_position, 2);

SAMPLER2D(s_ao, 3);
SAMPLER2D(s_metalRoughness, 4);
SAMPLER2D(s_emissive, 5);
SAMPLER2D(s_depth,  6);

#define PI 3.141592653589793

uniform vec3 u_viewPos;

uniform vec3 u_lightPosition;
uniform vec3 u_lightColor;
uniform vec4 u_lightTypeParams;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

void main()
{	
	// ========= Textures ========
	vec3 albedo         = toLinear(texture2D(s_albedo, v_texcoord0).rgb);
	vec3 metalRoughness = texture2D(s_metalRoughness, v_texcoord0).rgb;
	vec3 position       = convertRGB2XYZ(texture2D(s_position, v_texcoord0).rgb);
	vec3 normal         = decodeNormalOctahedron(texture2D(s_normal, v_texcoord0).rg);
	vec4 depth          = texture2D(s_depth, v_texcoord0);
	
	// ========= PBR Data ========
	vec3 emissive   = toLinear(texture2D(s_emissive, v_texcoord0).rgb);
	float ao        = texture2D(s_ao, v_texcoord0).r;
	float metallic  = metalRoughness.b;
	float roughness = metalRoughness.g;

	// ========= Lighting =========
	vec3 lighting = vec3(0.0, 0.0, 0.0);

	// pre-compute some math
	vec3 lightDir = normalize(u_lightPosition - position);
	vec3 viewDir = normalize(u_viewPos - position);
	vec3 reflectDir = reflect(-lightDir, normal);  
	vec3 lightDirH = normalize(viewDir + lightDir);

	vec3 radiance = vec3(0.0, 0.0, 0.0);

	// light type
	if (u_lightTypeParams.x == 0.0) {
		// Directional light

		radiance = u_lightColor;

	} else if (u_lightTypeParams.x == 1.0) {
		// Point light

		// attenuation
		float distance    = length(lightDir);
		float attenuation = 1.0 - smoothstep(u_lightTypeParams[2], 1.0, distance / u_lightTypeParams[1]);
		radiance     = u_lightColor * attenuation; 

	}

	// BRDF math shit
	vec3 F0 = vec3(0.04, 0.04, 0.04); 
	F0      = mix(F0, albedo, metallic);
	vec3 F  = fresnelSchlick(max(dot(lightDirH, viewDir), 0.0), F0);

	float NDF = DistributionGGX(normal, lightDirH, roughness);       
	float G   = GeometrySmith(normal, viewDir, lightDir, roughness);  

	vec3 numerator    = NDF * G * F;
	float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0)  + 0.0001;
	vec3 specular     = numerator / denominator;  

	vec3 kS = F;
	vec3 kD = vec3(1.0, 1.0, 1.0) - kS;
	
	kD *= 1.0 - metallic;	
  
    float NdotL = max(dot(normal, lightDir), 0.0);        
    lighting += (kD * albedo / PI + specular) * radiance * NdotL;

	vec3 ambient = vec3(0.1, 0.1, 0.1) * albedo * ao;
	vec3 color   = ambient + lighting + emissive;  

	gl_FragColor = vec4(color, 1.0);
}