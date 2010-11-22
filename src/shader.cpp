#include "shader.h"

CShader::CShader():
	m_ids(std::vector<GLuint>()),
	m_program(0), m_source("")
{
	m_program = glCreateProgram();
}

void CShader::Add(const string &fname, GLenum type)
{
	GLuint id = glCreateShader(type);

	Load(fname);
	//DEBUG
	// std::cout << m_source;
	
	const char *tmp = m_source.c_str();
	glShaderSource(id, 1, &tmp, NULL);
	glCompileShader(id);
	glAttachShader(m_program, id);
	
	m_ids.push_back(id);
}

void CShader::Link()
{
	glLinkProgram(m_program);
	PrintInfoLog();
}

CShader::~CShader()
{
	std::vector<GLuint>::iterator it = m_ids.begin();
	for (; it != m_ids.end(); ++it)
		glDeleteShader(*it);
	glDeleteProgram(m_program);
}

void CShader::PrintInfoLog()
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

	std::vector<GLuint>::iterator it = m_ids.begin();
	for (; it != m_ids.end(); ++it) {
		glGetShaderiv(*it, GL_INFO_LOG_LENGTH,&infologLength);

		if (infologLength > 0)
		{
			infoLog = new char[infologLength];

			glGetShaderInfoLog(*it, infologLength, &charsWritten, infoLog);
			std::cout << "shader " << *it << " info log" << std::endl;
			std::cout << infoLog;

			glGetProgramInfoLog(*it, infologLength, &charsWritten, infoLog);
			std::cout << infoLog;

			delete [] infoLog;
		}
	}
}

void CShader::Use()
{
	glUseProgram(m_program);
}

void CShader::Load(const string &fname)
{
	int length;
	char *buf;
	std::ifstream fp(fname.c_str());

	if (!fp) {
		m_source = "";
		return;
	}

	fp.seekg(0, std::ios::end);
	length = fp.tellg();
	fp.seekg(0, std::ios::beg);

	buf = new char[length + 1];
	fp.read(buf, length);
	buf[length] = 0;

	m_source = buf;

	delete [] buf;
	fp.close();
}
