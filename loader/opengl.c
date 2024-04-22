
typedef void type_glActiveTexture(GLenum texture);
typedef void type_glAttachShader(GLuint program, GLuint shader);
typedef void type_glBindBuffer(GLenum target, GLuint buffer);
typedef void type_glBindTexture(GLenum target, GLuint texture);
typedef void type_glBindVertexArray(GLuint array);
typedef void type_glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
typedef void type_glClear(GLbitfield mask);
typedef void type_glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void type_glCompileShader(GLuint shader);
typedef GLuint type_glCreateProgram(void);
typedef GLuint type_glCreateShader(GLenum type);
typedef void type_glDeleteShader(GLuint shader);
typedef void type_glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
typedef void type_glEnableVertexAttribArray(GLuint index);
typedef void type_glGenBuffers(GLsizei n, GLuint *buffers);
typedef void type_glGenTextures(GLsizei n, GLuint *textures);
typedef void type_glGenVertexArrays(GLsizei n, GLuint *arrays);
typedef void type_glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
typedef void type_glGetShaderiv(GLuint shader, GLenum pname, GLint *params);
typedef GLint type_glGetUniformLocation(GLuint program, const GLchar *name);
typedef void type_glLinkProgram(GLuint program);
typedef void type_glShaderSource(GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
typedef void type_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *data);
typedef void type_glTexParameteri(GLenum target, GLenum pname, GLint param);
typedef void type_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *data);
typedef void type_glUniform1f(GLint location, GLfloat v0);
typedef void type_glUniform1i(GLint location, GLint v0);
typedef void type_glUniform2f(GLint location, GLfloat v0, GLfloat v1);
typedef void type_glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void type_glUseProgram(GLuint program);
typedef void type_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
typedef void type_glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

#define OpenGLFunction(Name) type_##Name *Name

struct opengl {
	OpenGLFunction(glActiveTexture);
	OpenGLFunction(glAttachShader);
	OpenGLFunction(glBindBuffer);
	OpenGLFunction(glBindTexture);
	OpenGLFunction(glBindVertexArray);
	OpenGLFunction(glBufferData);
	OpenGLFunction(glClear);
	OpenGLFunction(glClearColor);
	OpenGLFunction(glCompileShader);
	OpenGLFunction(glCreateProgram);
	OpenGLFunction(glCreateShader);
	OpenGLFunction(glDeleteShader);
	OpenGLFunction(glDrawElements);
	OpenGLFunction(glEnableVertexAttribArray);
	OpenGLFunction(glGenBuffers);
	OpenGLFunction(glGenTextures);
	OpenGLFunction(glGenVertexArrays);
	OpenGLFunction(glGetShaderInfoLog);
	OpenGLFunction(glGetShaderiv);
	OpenGLFunction(glGetUniformLocation);
	OpenGLFunction(glLinkProgram);
	OpenGLFunction(glShaderSource);
	OpenGLFunction(glTexImage2D);
	OpenGLFunction(glTexParameteri);
	OpenGLFunction(glTexSubImage2D);
	OpenGLFunction(glUniform1f);
	OpenGLFunction(glUniform1i);
	OpenGLFunction(glUniform2f);
	OpenGLFunction(glUniform4f);
	OpenGLFunction(glUseProgram);
	OpenGLFunction(glVertexAttribPointer);
	OpenGLFunction(glViewport);
};
struct opengl opengl;

#if defined(_WIN32)
void *get_any_gl_address(const char *name) {
	void *p = (void *)wglGetProcAddress(name);
	if(p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1)) {
		HMODULE module = LoadLibraryA("opengl32.dll");
		p = (void *)GetProcAddress(module, name);
	}
	return p;
}

// #define Win32GetOpenGLFunction(Name) opengl->Name = (type_##Name *)wglGetProcAddress(#Name)
#define Win32GetOpenGLFunction(Name) opengl->Name = (type_##Name *)get_any_gl_address(#Name)

void gl_init(struct opengl *opengl) {
	printf("wglGetProcAddress %p\n", wglGetProcAddress);
	Win32GetOpenGLFunction(glActiveTexture);
	Win32GetOpenGLFunction(glAttachShader);
	Win32GetOpenGLFunction(glBindBuffer);
	Win32GetOpenGLFunction(glBindTexture);
	Win32GetOpenGLFunction(glBindVertexArray);
	Win32GetOpenGLFunction(glBufferData);
	Win32GetOpenGLFunction(glClear);
	Win32GetOpenGLFunction(glClearColor);
	Win32GetOpenGLFunction(glCompileShader);
	Win32GetOpenGLFunction(glCreateProgram);
	Win32GetOpenGLFunction(glCreateShader);
	Win32GetOpenGLFunction(glDeleteShader);
	Win32GetOpenGLFunction(glDrawElements);
	Win32GetOpenGLFunction(glEnableVertexAttribArray);
	Win32GetOpenGLFunction(glGenBuffers);
	Win32GetOpenGLFunction(glGenTextures);
	Win32GetOpenGLFunction(glGenVertexArrays);
	Win32GetOpenGLFunction(glGetShaderInfoLog);
	Win32GetOpenGLFunction(glGetShaderiv);
	Win32GetOpenGLFunction(glGetUniformLocation);
	Win32GetOpenGLFunction(glLinkProgram);
	Win32GetOpenGLFunction(glShaderSource);
	Win32GetOpenGLFunction(glTexImage2D);
	Win32GetOpenGLFunction(glTexParameteri);
	Win32GetOpenGLFunction(glTexSubImage2D);
	Win32GetOpenGLFunction(glUniform1f);
	Win32GetOpenGLFunction(glUniform1i);
	Win32GetOpenGLFunction(glUniform2f);
	Win32GetOpenGLFunction(glUniform4f);
	Win32GetOpenGLFunction(glUseProgram);
	Win32GetOpenGLFunction(glVertexAttribPointer);
	Win32GetOpenGLFunction(glViewport);
};

#elif defined(__linux__)

#define LinuxGetOpenGLFunction(Name) opengl->Name = (type_##Name *)glXGetProcAddress((GLubyte *) #Name)

void gl_init(struct opengl *opengl) {
	LinuxGetOpenGLFunction(glActiveTexture);
	LinuxGetOpenGLFunction(glAttachShader);
	LinuxGetOpenGLFunction(glBindBuffer);
	LinuxGetOpenGLFunction(glBindTexture);
	LinuxGetOpenGLFunction(glBindVertexArray);
	LinuxGetOpenGLFunction(glBufferData);
	LinuxGetOpenGLFunction(glClear);
	LinuxGetOpenGLFunction(glClearColor);
	LinuxGetOpenGLFunction(glCompileShader);
	LinuxGetOpenGLFunction(glCreateProgram);
	LinuxGetOpenGLFunction(glCreateShader);
	LinuxGetOpenGLFunction(glDeleteShader);
	LinuxGetOpenGLFunction(glDrawElements);
	LinuxGetOpenGLFunction(glEnableVertexAttribArray);
	LinuxGetOpenGLFunction(glGenBuffers);
	LinuxGetOpenGLFunction(glGenTextures);
	LinuxGetOpenGLFunction(glGenVertexArrays);
	LinuxGetOpenGLFunction(glGetShaderInfoLog);
	LinuxGetOpenGLFunction(glGetShaderiv);
	LinuxGetOpenGLFunction(glGetUniformLocation);
	LinuxGetOpenGLFunction(glLinkProgram);
	LinuxGetOpenGLFunction(glShaderSource);
	LinuxGetOpenGLFunction(glTexImage2D);
	LinuxGetOpenGLFunction(glTexParameteri);
	LinuxGetOpenGLFunction(glTexSubImage2D);
	LinuxGetOpenGLFunction(glUniform1f);
	LinuxGetOpenGLFunction(glUniform1i);
	LinuxGetOpenGLFunction(glUniform2f);
	LinuxGetOpenGLFunction(glUniform4f);
	LinuxGetOpenGLFunction(glUseProgram);
	LinuxGetOpenGLFunction(glVertexAttribPointer);
	LinuxGetOpenGLFunction(glViewport);
}

#endif

#define glActiveTexture					opengl.glActiveTexture
#define glAttachShader					opengl.glAttachShader
#define glBindBuffer						opengl.glBindBuffer
#define glBindTexture					opengl.glBindTexture
#define glBindVertexArray				opengl.glBindVertexArray
#define glBufferData						opengl.glBufferData
#define glClear							opengl.glClear
#define glClearColor						opengl.glClearColor
#define glCompileShader					opengl.glCompileShader
#define glCreateProgram					opengl.glCreateProgram
#define glCreateShader					opengl.glCreateShader
#define glDeleteShader					opengl.glDeleteShader
#define glDrawElements					opengl.glDrawElements
#define glEnableVertexAttribArray	opengl.glEnableVertexAttribArray
#define glGenBuffers						opengl.glGenBuffers
#define glGenTextures					opengl.glGenTextures
#define glGenVertexArrays				opengl.glGenVertexArrays
#define glGetShaderInfoLog				opengl.glGetShaderInfoLog
#define glGetShaderiv					opengl.glGetShaderiv
#define glGetUniformLocation			opengl.glGetUniformLocation
#define glLinkProgram					opengl.glLinkProgram
#define glShaderSource					opengl.glShaderSource
#define glTexImage2D						opengl.glTexImage2D
#define glTexParameteri					opengl.glTexParameteri
#define glTexSubImage2D					opengl.glTexSubImage2D
#define glUniform1f						opengl.glUniform1f
#define glUniform1i						opengl.glUniform1i
#define glUniform2f						opengl.glUniform2f
#define glUniform4f						opengl.glUniform4f
#define glUseProgram						opengl.glUseProgram
#define glVertexAttribPointer			opengl.glVertexAttribPointer
#define glViewport						opengl.glViewport
