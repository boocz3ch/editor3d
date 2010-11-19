#ifndef _SHADER_H
#define _SHADER_H

#include <iostream>
#include <string>
#include <fstream>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

using namespace std;

struct Exception {
	std::string msg;
};

class CShader {
	GLuint m_id;
	GLuint m_program;
	std::string m_source;
public:
	CShader(const string &, GLenum);
	~CShader();
	void Load(const string &);
	void PrintInfoLog();
	GLuint GetID() { return m_id; }
	GLuint GetProgram() { return m_program; }
	void Use();
};


#endif
