#include "shader.h"

CShader::CShader(const string &fname, GLenum type)
{
	const char *tmp;
	m_id = glCreateShader(type);
	// m_frag = glCreateShader(GL_FRAGMENT_SHADER);

	Load(fname);
	//DEBUG
	// std::cout << m_source;
	
	tmp = m_source.c_str();
	glShaderSource(m_id, 1, &tmp, NULL);
	glCompileShader(m_id);
	// PrintInfoLog(m_id);
	
	m_program = glCreateProgram();
	// glAttachShader(m_program, m_vert);
	glAttachShader(m_program, m_id);
	glLinkProgram(m_program);
	// printProgramInfoLog(m_program);
	// glUseProgram(m_program);
	
	PrintInfoLog();
/*
 *     tmp = m_source.c_str();
 *     glShaderSource(m_vert, 1, &tmp, NULL);
 *     //DEBUG
 *     // std::cout << "VERT:\n" << m_source;
 *     load_source(f_fname);
 *     //DEBUG
 *     // std::cout << "FRAG:\n" << m_source;
 *     tmp = m_source.c_str();
 *     glShaderSource(m_frag, 1, &tmp, NULL);
 * 
 *     glCompileShader(m_vert);
 *     glCompileShader(m_frag);
 * 
 *     printShaderInfoLog(m_vert);
 *     printShaderInfoLog(m_frag);
 * 
 *     m_program = glCreateProgram();
 *     glAttachShader(m_program, m_vert);
 *     glAttachShader(m_program, m_frag);
 *     glLinkProgram(m_program);
 *     printProgramInfoLog(m_program);
 *     glUseProgram(m_program);
 */

	// glGetUniformLocation(m_loc, "time");

	// std::cout << m_program << std::endl;
}

CShader::~CShader()
{
	// glDestroyProgram(m_program);
	// glDeleteShader(m_...);
	// glDeleteProgram(m_program);
}

void CShader::PrintInfoLog()
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

	glGetShaderiv(m_id, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0)
    {
        infoLog = new char[infologLength];
        
		glGetShaderInfoLog(m_id, infologLength, &charsWritten, infoLog);
		std::cout << "shader " << m_id << " info log" << std::endl;
		std::cout << infoLog;
		
		glGetProgramInfoLog(m_id, infologLength, &charsWritten, infoLog);
		std::cout << infoLog;
		
        delete [] infoLog;
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
