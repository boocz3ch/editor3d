#version 330

uniform mat4 projection;
uniform mat4 modelview;
uniform float t;

in vec3 vertex;
in vec3 in_color;
in vec2 in_texcoord;

// picked vertex
uniform vec3 in_picked;
// pass picked to fragment shader
out vec3 out_picked;

out vec3 out_vertex;
out vec3 color;
out vec2 texcoord;

void main()
{
	// pass data to frag shader
	color = in_color;
	texcoord = in_texcoord;
	out_vertex = vertex;
	out_picked = in_picked;

	// out it goes
	gl_Position = projection * modelview * vec4(vertex, 1.0);
}
