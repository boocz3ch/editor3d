#version 140

uniform vec3 MyTime;
out vec3 c;

uniform Transformation {
    mat4 projection_matrix;
    mat4 modelview_matrix;
};
 
in vec3 vertex;
vec3 pos;
 
void main()
{
	c = MyTime;
    gl_Position = projection_matrix * modelview_matrix * vec4(vec3(0,0,0), 1.0);
   // gl_Position = vec4(5.0,5.0,5.0,1.0);
}

