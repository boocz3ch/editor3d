#version 330

uniform sampler2D tex;
//layout (location = 0, index = 0) out vec4 fragColor;

in vec2 texcoord;

in vec3 out_picked;
in vec3 out_vertex;

out vec4 final_color;

const int tiles = 2;

void main()
{
	vec4 shade = vec4(1.0);
	float len = length(out_picked.xz - out_vertex.xz);
	// highlight mouse location
	if (len <= 3.0)
		shade = vec4(0.4, 0.6, 1.0, 1.0);

	final_color = shade * texture2D(tex, texcoord);

	// show grid
	for (int i = 1; i < tiles; i++) {
		if (out_vertex.x >= 51.0*i && out_vertex.x <= 51.4*i)
			final_color = vec4(0.6);
		if (out_vertex.z >= 51.0*i && out_vertex.z <= 51.4*i)
			final_color = vec4(0.6);
	}
}


