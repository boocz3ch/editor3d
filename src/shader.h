#ifndef _SHADER_H_
#define _SHADER_H_
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

class CShader {
	std::vector<GLuint> m_ids;
	GLuint m_program;
	std::string m_source;
	
	void Load(const std::string &);
public:
	CShader();
	~CShader();
	void Add(const std::string &fname, GLenum);
	void Link();
	void PrintInfoLog();
	GLuint GetProgram() { return m_program; }
	void Use();
};

#endif
