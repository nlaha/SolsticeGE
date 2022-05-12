$input v_wpos, v_view, v_normal, v_tangent, v_bitangent, v_texcoord0, v_model

#include <bgfx_shader.sh>
#include "shaderlib.sh"
#include "common.sh"

SAMPLER2D(s_texColor,  0);
SAMPLER2D(s_texNormal, 1);

void main()
{	
	// get normal map
	vec3 normalMap = texture2D(s_texNormal, v_texcoord0).rgb;
    normalMap = normalize(normalMap * 2.0 - 1.0); // fix map range
	
	mat3 tbn = transpose(mat3(
        v_tangent,
        v_bitangent,
        v_normal
    ));

	vec3 normal = normalize(mul(tbn, normalMap) );

	// ==== output ====
	gl_FragData[0] = texture2D(s_texColor, v_texcoord0); // albedo
	gl_FragData[1] = vec4(encodeNormalUint(normal), 0.0); // normal
	gl_FragData[2] = vec4(convertXYZ2RGB(v_wpos), 0.0); // position
}