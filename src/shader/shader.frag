//#version 150
#version 330

uniform sampler2D tex;
//layout (location = 0, index = 0) out vec4 fragColor;

in vec3 color;
in vec2 texcoord;
out vec4 final_color;

void main()
{
	// modrej filter
	final_color = vec4(color, 1.0) * texture2D(tex, texcoord);
}


