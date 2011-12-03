#version 330

uniform mat4 projection;
uniform mat4 modelview;

in vec3 vertex;
in vec2 in_texcoord;
in vec3 in_normal;

// picked vertex
uniform vec3 in_picked;
// pass picked to fragment shader
out vec3 out_picked;

out vec3 out_vertex;
out vec2 texcoord;
out vec3 out_normal;

// light
out vec3 lightdir;
out vec3 eyevec;
uniform vec3 lightpos;

void main()
{
	// light
	/* vec4 lightpos = modelview * vec4(79.0*1.5, 94.0, 70.0*1.5, 1.0); */
	vec4 lightposeye = modelview * vec4(lightpos, 1.0);
	vec4 vvertex = modelview * vec4(vertex, 1.0);
	lightdir = vec3(lightposeye.xyz - vvertex.xyz);
	eyevec = -vvertex.xyz;

	// pass data to frag shader
	texcoord = in_texcoord;
	out_vertex = vertex;
	out_picked = in_picked;
	mat4 G = transpose(inverse(modelview));
	// eye coord normal
	out_normal = vec3(G * vec4(in_normal, 0.0));

	// out it goes
	gl_Position = projection * modelview * vec4(vertex, 1.0);
}
