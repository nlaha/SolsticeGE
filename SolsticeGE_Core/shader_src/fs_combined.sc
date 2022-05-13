$input v_texcoord0

#include <bgfx_shader.sh>
#include "shaderlib.sh"

SAMPLER2D(s_light,  0);

void main()
{
	vec4 light   = toLinear(texture2D(s_light,  v_texcoord0) );
	gl_FragColor = toGamma(light);
}