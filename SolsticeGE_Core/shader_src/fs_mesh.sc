$input v_color0
$input v_normal

#include <bgfx_shader.sh>

void main()
{
	gl_FragColor = vec4(v_normal.x, v_normal.y, v_normal.z, 1.0);
}