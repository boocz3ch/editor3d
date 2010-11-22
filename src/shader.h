#ifndef _SHADER_H
#define _SHADER_H

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

using namespace std;

struct Exception {
	std::string msg;
};

class CShader {
	std::vector<GLuint> m_ids;
	GLuint m_program;
	std::string m_source;
	
	void Load(const string &);
public:
	CShader();
	~CShader();
	void Add(const string &fname, GLenum);
	void Link();
	void PrintInfoLog();
	GLuint GetProgram() { return m_program; }
	void Use();
};

#endif
