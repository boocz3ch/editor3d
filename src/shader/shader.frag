#version 330

uniform sampler2D tex;
//layout (location = 0, index = 0) out vec4 fragColor;

in vec3 color;
in vec2 texcoord;

in vec3 out_picked;
in vec3 out_vertex;

out vec4 final_color;

void main()
{
	vec4 shade = vec4(1.0);
	float len = length(out_picked.xz - out_vertex.xz);
	// highlight mouse location
	if (len <= 3.0)
		shade = vec4(0.4, 0.6, 1.0, 1.0);

	final_color = vec4(color, 1.0) * shade * texture2D(tex, texcoord);
}


