$input a_position, a_normal, a_tangent, a_bitangent, a_texcoord0
$output v_wpos, v_view, v_normal, v_tangent, v_bitangent, v_texcoord0, v_model

#include <bgfx_shader.sh>

void main()
{
	// ===== convert to world space =====
	vec3 wpos = mul(u_model[0], vec4(a_position, 1.0) ).xyz;
	vec3 wnormal = mul(u_model[0], vec4(a_normal.xyz, 0.0) ).xyz;
	vec3 wtangent = mul(u_model[0], vec4(a_tangent.xyz, 0.0) ).xyz;
	vec3 wbitangent = mul(u_model[0], vec4(a_bitangent.xyz, 0.0) ).xyz;

	// ====== Make TBN matrix ======
	mat3 tbn = transpose(mat3(
        wtangent,
        wbitangent,
        wnormal
    ));

	vec3 view = mul(u_view, vec4(wpos, 0.0) ).xyz;

	// ===== Send to fragment shader =====
	v_wpos = wpos;
	v_view = mul(view, tbn);
	v_normal    = wnormal;
	v_tangent   = wtangent;
	v_bitangent = wbitangent;
	v_texcoord0 = a_texcoord0;

	gl_Position = mul(u_viewProj, vec4(wpos, 1.0) );
}