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

uniform vec3 u_viewPos;

uniform vec3 u_lightPosition;
uniform vec3 u_lightColor;
uniform vec4 u_lightTypeParams;

struct CommonData {
	vec3 albedo;
	vec3 normal;
	vec3 viewDir;
	vec3 reflectDir;
	vec3 lightDir;
};

vec3 calcDiffuse(CommonData cdata)
{
	return max(dot(cdata.normal, cdata.lightDir), 0.0) 
			* u_lightColor;
}

vec3 calcSpecular(CommonData cdata, float specular)
{
	return pow(max(dot(cdata.viewDir, cdata.reflectDir), 0.0), 32) 
			* u_lightColor * specular;
}

void main()
{	
	// ========= Textures ========
	vec4 albedo = texture2D(s_albedo, v_texcoord0);
	
	float ao = texture2D(s_ao, v_texcoord0).r;
	vec2 metalRoughness = texture2D(s_metalRoughness, v_texcoord0).rg;
	vec3 emissive = texture2D(s_emissive, v_texcoord0).rgb;

	vec3 position = convertRGB2XYZ(texture2D(s_position, v_texcoord0).rgb);
	vec3 normal   = decodeNormalOctahedron(texture2D(s_normal,   v_texcoord0).rg);
	vec4 depth    = texture2D(s_depth,    v_texcoord0);

	float specular_strength = 1.0 - metalRoughness.g;

	// ========= Lighting =========
	// ambient
	vec3 lighting = 0.0;
	vec3 ambient = 0.1;
	lighting += ambient;

	// pre-compute some math
	vec3 lightDir = normalize(u_lightPosition - position);
	vec3 viewDir = normalize(u_viewPos - position);
	vec3 reflectDir = reflect(-lightDir, normal);  

	CommonData cdata;
	cdata.albedo = albedo.rgb;
	cdata.normal = normal;
	cdata.viewDir = viewDir;
	cdata.reflectDir = reflectDir;
	cdata.lightDir = lightDir;

	// light type
	if (u_lightTypeParams.x == 0.0) {
		// Directional light

		// diffuse
		vec3 diffuse = calcDiffuse(cdata);
		lighting += diffuse;

		// specular
		vec3 specular = calcSpecular(cdata, specular_strength);
		lighting += specular;

	} else if (u_lightTypeParams.x == 1.0) {
		// Point light

		// diffuse
		vec3 diffuse = calcDiffuse(cdata);
		lighting += diffuse;

		// specular
		vec3 specular = calcSpecular(cdata, specular_strength);
		lighting += specular;

		// attenuation
		float distance    = length(u_lightPosition - position);
		float attenuation = 1.0 - smoothstep(u_lightTypeParams[2], 1.0, distance / u_lightTypeParams[1]);
		lighting *= attenuation;

	}

	gl_FragColor = vec4(lighting * albedo.rgb * ao, 1.0);
}