#include <ting/Array.hpp>

#include "Shader.hpp"
//#include "GLWindow.hpp"


using namespace ting;
using namespace tride;



Shader::Shader() :
		vertexShader(0),
		fragmentShader(0),
		program(0)
{}



Shader::~Shader(){
	//make sure the shader objects are created before deleting them
	ASSERT(this->vertexShader != 0)
	ASSERT(this->fragmentShader != 0)
	ASSERT(this->program != 0)

	glDeleteProgram(this->program);
	glDeleteShader(this->fragmentShader);
	glDeleteShader(this->vertexShader);
}



bool Shader::CheckForCompileErrors(GLuint shader){
	GLint value = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &value);
	if(value == 0){ //if not compiled
		GLint logLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		if(logLen > 1){//1 char is a terminating 0
			ting::Array<char> log(logLen);
			GLint len;
			glGetShaderInfoLog(shader, log.Size(), &len, &log[0]);
			TRACE(<< "===Compile log===\n" << log.Begin() << std::endl)
		}else{
			TRACE(<< "Shader::CheckForCompileErrors(): log length is 0" << std::endl)
		}
		return true;
	}
	return false;
}



bool Shader::CheckForLinkErrors(GLuint program){
	GLint value = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &value);
	if(value == 0){ //if not linked
		GLint logLen = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
		if(logLen > 1){ //1 is for terminating 0 character.
			ting::Array<char> log(logLen);
			GLint len;
			glGetProgramInfoLog(program, log.Size(), &len, &log[0]);
			TRACE(<< "===Link log===\n" << log.Begin() << std::endl)
		}else{
			TRACE(<< "Shader::CheckForLinkErrors(): log length is 0" << std::endl)
		}
		return true;
	}
	return false;
}



static Vec2f quadTriangles[] = {
	Vec2f(-1, -1), Vec2f(-1, 1), Vec2f(1, 1), Vec2f(1, -1)
};



void Shader::DrawQuad(GLenum mode){
	this->EnablePositionPointer();
	this->SetPositionPointer(quadTriangles);
	this->DrawArrays(mode, 4);
}



static Vec2f quad01Triangles[] = {
	Vec2f(0, 0), Vec2f(0, 1), Vec2f(1, 1), Vec2f(1, 0)
};



void Shader::DrawQuad01(GLenum mode){
	this->EnablePositionPointer();
	this->SetPositionPointer(quad01Triangles);
	this->DrawArrays(mode, 4);
}



static Vec2f quadTexCoords[] = {
	Vec2f(0, 0), Vec2f(0, 1), Vec2f(1, 1), Vec2f(1, 0)
};



void TexturingShader::DrawQuad(GLenum mode){
	this->EnableTexCoordPointer();
	this->SetTexCoordPointer(quadTexCoords);
	this->Shader::DrawQuad(mode);
}



void TexturingShader::DrawQuad01(GLenum mode){
	this->EnableTexCoordPointer();
	this->SetTexCoordPointer(quadTexCoords);
	this->Shader::DrawQuad01(mode);
}



SimpleTexturingShader::SimpleTexturingShader(){
	this->vertexShader = glCreateShader(GL_VERTEX_SHADER);
	const char *vsrc[1] = {
		"attribute highp vec4 vertex;\n"
		"attribute highp vec2 textureCoord;\n"
		"uniform highp mat4 matrix;\n"
		"varying highp vec2 texCoord;\n"
		"void main(void){\n"
			"gl_Position = matrix * vertex;\n"
			"texCoord = textureCoord;\n"
		"}\n"
	};
	glShaderSource(this->vertexShader, 1, vsrc, 0);
	glCompileShader(this->vertexShader);
	if(this->CheckForCompileErrors(this->vertexShader)){
		TRACE(<< "SimpleTexturingShader: Error while compiling:\n" << vsrc[0] << std::endl)
		throw ting::Exc("Error compiling vertex shader");
	}

	this->fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	const char *fsrc[1] = {
		"uniform sampler2D texNumber;\n"
		"varying highp vec2 texCoord;\n"
		"void main(void){\n"
			"gl_FragColor = texture2D(texNumber, texCoord);\n"
		"}\n"
	};
	glShaderSource(this->fragmentShader, 1, fsrc, 0);
	glCompileShader(this->fragmentShader);
	if(this->CheckForCompileErrors(this->fragmentShader)){
		TRACE(<< "SimpleTexturingShader: Error while compiling:\n" << fsrc[0] << std::endl)
		throw ting::Exc("Error compiling fragment shader");
	}

	this->program = glCreateProgram();
	glAttachShader(this->program, this->vertexShader);
	glAttachShader(this->program, this->fragmentShader);
	glLinkProgram(this->program);
	if(this->CheckForLinkErrors(this->program)){
		TRACE(<< "SimpleTexturingShader: Error while linking shader program" << std::endl)
		throw ting::Exc("Error linking shader program");
	}

	this->positionAttr = glGetAttribLocation(this->program, "vertex");
	this->texCoordAttr = glGetAttribLocation(this->program, "textureCoord");

	this->matrixUniform = glGetUniformLocation(this->program, "matrix");
	this->texNumberUniform = glGetUniformLocation(this->program, "texNumber");
}



ColoringTexturingShader::ColoringTexturingShader(){
	this->vertexShader = glCreateShader(GL_VERTEX_SHADER);
	const char *vsrc[1] = {
		"attribute highp vec4 vertex;\n"
		"attribute highp vec2 textureCoord;\n"
		"attribute lowp vec4 vertexColor;\n"
		"uniform highp mat4 matrix;\n"
		"varying lowp vec4 color;\n"
		"varying highp vec2 texCoord;\n"
		"void main(void){\n"
			"gl_Position = matrix * vertex;\n"
			"color = vertexColor;\n"
			"texCoord = textureCoord;\n"
		"}\n"
	};
	glShaderSource(this->vertexShader, 1, vsrc, 0);
	glCompileShader(this->vertexShader);
	if(this->CheckForCompileErrors(this->vertexShader)){
		TRACE(<< "Error while compiling:\n" << vsrc[0] << std::endl)
		throw ting::Exc("Error compiling vertex shader");
	}

	this->fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	const char *fsrc[1] = {
		"uniform sampler2D texNumber;\n"
		"varying lowp vec4 color;\n"
		"varying highp vec2 texCoord;\n"
		"void main(void){\n"
			"highp vec4 texCol = texture2D(texNumber, texCoord);\n"
//			"gl_FragColor = color + (texCol - color) * texCol.a;\n"
			"gl_FragColor = vec4(color.rgb + (texCol.rgb - color.rgb) * texCol.a, texCol.a);\n"
		"}\n"
	};
	glShaderSource(this->fragmentShader, 1, fsrc, 0);
	glCompileShader(this->fragmentShader);
	if(this->CheckForCompileErrors(this->fragmentShader)){
		TRACE(<< "Error while compiling:\n" << fsrc[0] << std::endl)
		throw ting::Exc("Error compiling fragment shader");
	}

	this->program = glCreateProgram();
	glAttachShader(this->program, this->vertexShader);
	glAttachShader(this->program, this->fragmentShader);
	glLinkProgram(this->program);
	if(this->CheckForLinkErrors(this->program)){
		TRACE(<< "Error while linking shader program" << std::endl)
		throw ting::Exc("Error linking shader program");
	}

	this->positionAttr = glGetAttribLocation(this->program, "vertex");
	this->texCoordAttr = glGetAttribLocation(this->program, "textureCoord");
	this->vertexColorAttr = glGetAttribLocation(this->program, "vertexColor");

	this->matrixUniform = glGetUniformLocation(this->program, "matrix");
	this->texNumberUniform = glGetUniformLocation(this->program, "texNumber");
}



SimpleColoringShader::SimpleColoringShader(){
	this->vertexShader = glCreateShader(GL_VERTEX_SHADER);
	const char *vsrc[1] = {
		"attribute highp vec4 vertex;\n"
		"attribute lowp vec4 vertexColor;\n"
		"uniform highp mat4 matrix;\n"
		"varying lowp vec4 color;\n"
		"void main(void){\n"
			"gl_Position = matrix * vertex;\n"
			"color = vertexColor;\n"
		"}\n"
	};

	glShaderSource(this->vertexShader, 1, vsrc, 0);
	glCompileShader(this->vertexShader);
	if(this->CheckForCompileErrors(this->vertexShader)){
		TRACE(<< "Error while compiling:\n" << vsrc[0] << std::endl)
		throw ting::Exc("Error compiling vertex shader");
	}

	this->fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	const char *fsrc[1] = {
		"varying lowp vec4 color;\n"
		"void main(void){\n"
			"gl_FragColor = color;\n"
		"}\n"
	};
	glShaderSource(this->fragmentShader, 1, fsrc, 0);
	glCompileShader(this->fragmentShader);
	if(this->CheckForCompileErrors(this->fragmentShader)){
		TRACE(<< "Error while compiling:\n" << fsrc[0] << std::endl)
		throw ting::Exc("Error compiling fragment shader");
	}

	this->program = glCreateProgram();
	glAttachShader(this->program, this->vertexShader);
	glAttachShader(this->program, this->fragmentShader);
	glLinkProgram(this->program);
	if(this->CheckForLinkErrors(this->program)){
		TRACE(<< "Error while linking shader program" << std::endl)
		throw ting::Exc("Error linking shader program");
	}

	this->positionAttr = glGetAttribLocation(this->program, "vertex");
	this->vertexColorAttr = glGetAttribLocation(this->program, "vertexColor");

	this->matrixUniform = glGetUniformLocation(this->program, "matrix");
}



ModulatingColoringTexturingShader::ModulatingColoringTexturingShader(){
	this->vertexShader = glCreateShader(GL_VERTEX_SHADER);
	const char *vsrc[1] = {
		"attribute highp vec4 vertex;\n"
		"attribute highp vec2 textureCoord;\n"
		"attribute lowp vec4 vertexColor;\n"
		"uniform highp mat4 matrix;\n"
		"varying lowp vec4 color;\n"
		"varying highp vec2 texCoord;\n"
		"void main(void){\n"
			"gl_Position = matrix * vertex;\n"
			"color = vertexColor;\n"
			"texCoord = textureCoord;\n"
		"}\n"
	};
	glShaderSource(this->vertexShader, 1, vsrc, 0);
	glCompileShader(this->vertexShader);
	if(this->CheckForCompileErrors(this->vertexShader)){
		TRACE(<< "Error while compiling:\n" << vsrc[0] << std::endl)
		throw ting::Exc("Error compiling vertex shader");
	}

	this->fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	const char *fsrc[1] = {
		"uniform sampler2D texNumber;\n"
		"varying lowp vec4 color;\n"
		"varying highp vec2 texCoord;\n"
		"void main(void){\n"
			"highp vec4 texCol = texture2D(texNumber, texCoord);\n"
			"gl_FragColor = texCol * color;\n" //component-wise multiplication
		"}\n"
	};
	glShaderSource(this->fragmentShader, 1, fsrc, 0);
	glCompileShader(this->fragmentShader);
	if(this->CheckForCompileErrors(this->fragmentShader)){
		TRACE(<< "Error while compiling:\n" << fsrc[0] << std::endl)
		throw ting::Exc("Error compiling fragment shader");
	}

	this->program = glCreateProgram();
	glAttachShader(this->program, this->vertexShader);
	glAttachShader(this->program, this->fragmentShader);
	glLinkProgram(this->program);
	if(this->CheckForLinkErrors(this->program)){
		TRACE(<< "Error while linking shader program" << std::endl)
		throw ting::Exc("Error linking shader program");
	}

	this->positionAttr = glGetAttribLocation(this->program, "vertex");
	this->texCoordAttr = glGetAttribLocation(this->program, "textureCoord");
	this->vertexColorAttr = glGetAttribLocation(this->program, "vertexColor");

	this->matrixUniform = glGetUniformLocation(this->program, "matrix");
	this->texNumberUniform = glGetUniformLocation(this->program, "texNumber");
}



SaturatingColoringTexturingShader::SaturatingColoringTexturingShader(){
	this->vertexShader = glCreateShader(GL_VERTEX_SHADER);
	const char *vsrc[1] = {
		"attribute highp vec4 vertex;\n"
		"attribute highp vec2 textureCoord;\n"
		"attribute lowp vec4 vertexColor;\n"
		"uniform highp mat4 matrix;\n"
		"varying lowp vec4 color;\n"
		"varying highp vec2 texCoord;\n"
		"void main(void){\n"
			"gl_Position = matrix * vertex;\n"
			"color = vertexColor;\n"
			"texCoord = textureCoord;\n"
		"}\n"
	};
	glShaderSource(this->vertexShader, 1, vsrc, 0);
	glCompileShader(this->vertexShader);
	if(this->CheckForCompileErrors(this->vertexShader)){
		TRACE(<< "Error while compiling:\n" << vsrc[0] << std::endl)
		throw ting::Exc("Error compiling vertex shader");
	}

	this->fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	const char *fsrc[1] = {
		"uniform sampler2D texNumber;\n"
		"varying lowp vec4 color;\n"
		"varying highp vec2 texCoord;\n"
		"void main(void){\n"
			"highp vec4 texCol = texture2D(texNumber, texCoord);\n"
			"gl_FragColor = texCol * (vec4(1.0) - color) + color;\n" //component-wise multiplication
		"}\n"
	};
	glShaderSource(this->fragmentShader, 1, fsrc, 0);
	glCompileShader(this->fragmentShader);
	if(this->CheckForCompileErrors(this->fragmentShader)){
		TRACE(<< "Error while compiling:\n" << fsrc[0] << std::endl)
		throw ting::Exc("Error compiling fragment shader");
	}

	this->program = glCreateProgram();
	glAttachShader(this->program, this->vertexShader);
	glAttachShader(this->program, this->fragmentShader);
	glLinkProgram(this->program);
	if(this->CheckForLinkErrors(this->program)){
		TRACE(<< "Error while linking shader program" << std::endl)
		throw ting::Exc("Error linking shader program");
	}

	this->positionAttr = glGetAttribLocation(this->program, "vertex");
	this->texCoordAttr = glGetAttribLocation(this->program, "textureCoord");
	this->vertexColorAttr = glGetAttribLocation(this->program, "vertexColor");

	this->matrixUniform = glGetUniformLocation(this->program, "matrix");
	this->texNumberUniform = glGetUniformLocation(this->program, "texNumber");
}


