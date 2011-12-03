#version 330

struct brush_t {
	bool circle;
	bool rect;
	int  size;
};
uniform brush_t brush;

uniform sampler2D tex;
//layout (location = 0, index = 0) out vec4 fragColor;

in vec2 texcoord;
uniform float show_texture;

in vec3 out_picked;
in vec3 out_vertex;
in vec3 out_normal;

out vec4 final_color;

const int tiles = 2;
const vec4 brush_color = vec4(0.4, 0.6, 1.0, 1.0);

// light
in vec3 lightdir;
in vec3 eyevec;
uniform float show_light;

void main()
{
	vec4 shade = vec4(1.0);
	float len = length(out_picked.xz - out_vertex.xz);
	float len_x = length(out_picked.x - out_vertex.x);
	float len_z = length(out_picked.z - out_vertex.z);

	// highlight mouse location
	if (brush.circle) {
		if (len <= brush.size)
			shade = brush_color;
	}
	else if (brush.rect) {
		if (len_x <= brush.size && len_z <= brush.size)
			shade = brush_color;
	}

	// light
	vec3 N = normalize(out_normal);
	vec3 L = normalize(lightdir);
	float NdotL = dot(N, L);
	vec3 c = vec3(1.0, 1.0, 1.0);
	
        const vec3 diffuse = vec3(0.9, 0.9, 0.9);
	const vec3 specular = vec3(1.0, 1.0, 1.0) - diffuse;
	/* const float shininess = 2.0; */
	const float shininess = 50.0;
	const float ka = 0.1;
	const float kd = 1.0;
	const float ks = 1.0;

	vec3 E = normalize(eyevec);
	vec3 R = reflect(-L, N);

	c *= ka;
	c += kd * diffuse * max(NdotL, ka);	
	c += ks * specular * pow(max(dot(R, E), 0.0), shininess);

	final_color = vec4(1.0, 1.0, 1.0, 1.0) * shade;

	if (show_light > 0.1)
		final_color *= vec4(c, 1.0);
	if (show_texture > 0.1)
		final_color *= texture2D(tex, texcoord);
		
	/* final_color = vec4(c, 1.0) * shade * texture2D(tex, texcoord); */
}


