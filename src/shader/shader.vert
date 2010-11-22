#version 330

uniform mat4 projection;
uniform mat4 modelview;
uniform float t;

in vec3 vertex;
in vec3 in_color;
in vec2 in_texcoord;

out vec3 color;
out vec2 texcoord;

void main()
{
	vec4 tmp = vec4(vertex, 1.0);
	//tmp.y += 1000.0;
	//tmp.x *= 0.1*t;
	//tmp.x += 8.8*sin(t);
	color = in_color;
	texcoord = in_texcoord;
	gl_Position = projection*modelview*tmp;
}
