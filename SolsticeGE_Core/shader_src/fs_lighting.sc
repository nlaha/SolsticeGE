$input v_texcoord0

#include <bgfx_shader.sh>
#include "shaderlib.sh"
#include "common.sh"

SAMPLER2D(s_albedo,  0);
SAMPLER2D(s_depth,  1);
SAMPLER2D(s_normal, 2);
SAMPLER2D(s_position, 3);

uniform vec3 u_viewPos;

uniform vec3 u_lightPositions[32];
uniform vec3 u_lightColors[32];

void main()
{	
	// ========= Textures ========
	vec4 albedo = texture2D(s_albedo, v_texcoord0);
	vec4 position = texture2D(s_position, v_texcoord0);
	vec3 normal   = decodeNormalUint(texture2D(s_normal,   v_texcoord0));
	vec4 depth    = texture2D(s_depth,    v_texcoord0);

    vec3 viewDir = normalize(u_viewPos - position);

	// ========= Lighting =========
	vec3 lighting = 0.0;
	vec3 ambient = 0.1 * albedo;
	lighting += ambient;

    for(int i = 0; i < 32; ++i)
    {
		// diffuse
		vec3 lightDir = normalize(u_lightPositions[i] - position);
		vec3 diffuse = max(dot(normal, lightDir), 0.0) * albedo * u_lightColors[i];
		lighting += diffuse;
	}

	gl_FragColor = vec4(lighting, 1.0);
}