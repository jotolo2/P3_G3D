#include "BOX.h"
#include "auxiliar.h"


#include <gl/glew.h>
#define SOLVE_FGLUT_WARNING
#include <gl/freeglut.h> 

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>


//////////////////////////////////////////////////////////////
// Datos que se almacenan en la memoria de la CPU
//////////////////////////////////////////////////////////////

//Matrices
glm::mat4	proj = glm::mat4(1.0f);
glm::mat4	view = glm::mat4(1.0f);
glm::mat4	model[3];
glm::vec4	lightPos;
glm::vec3	lightAmb;
glm::vec3	lightDif;
glm::vec3	lightSpec;

//////////////////////////////////////////////////////////////
// Variables que nos dan acceso a Objetos OpenGL
//////////////////////////////////////////////////////////////
//Por definir
unsigned int vshader[3];
unsigned int fshader[3];
unsigned int program[3];


//Variables Uniform
struct Uniforms {
	int uModelViewMat;
	int uModelViewProjMat;
	int uNormalMat;
	int uColorTex;
	int uEmiTex;
	int uLightPos;
	int uLightAmb;
	int uLightDif;
	int uLightSpec;
};
Uniforms uniforms[3];

//Texturas
unsigned int colorTexId1, colorTexId2, emiTexId;

//Atributos
struct Attributes {
	int inPos;
	int inColor;
	int inNormal;
	int inTexCoord;
};
Attributes attributes[3];


//VAO
unsigned int vao[3];
//VBOs que forman parte del objeto
struct VBOS {
	unsigned int posVBO;
	unsigned int colorVBO;
	unsigned int normalVBO;
	unsigned int texCoordVBO;
	unsigned int triangleIndexVBO;
};
VBOS vbos[3];




//////////////////////////////////////////////////////////////
// Funciones auxiliares
//////////////////////////////////////////////////////////////
//!!Por implementar

//Declaración de CB
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);

//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initShader(const char *vname, const char *fname, unsigned int &, unsigned int &, unsigned int &, size_t);
void initObj(size_t);
void destroy();


//Carga el shader indicado, devuele el ID del shader
//!Por implementar
GLuint loadShader(const char *fileName, GLenum type);

//Crea una textura, la configura, la sube a OpenGL, 
//y devuelve el identificador de la textura 
//!!Por implementar
unsigned int loadTex(const char *fileName);


int main(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));// acentos ;)

	initContext(argc, argv);
	initOGL();

	initShader("../shaders_P3/shader.v1.vert", "../shaders_P3/shader.v1.frag", program[0], vshader[0], fshader[0], 0);
	initShader("../shaders_P3/shader.v2.vert", "../shaders_P3/shader.v2.frag", program[1], vshader[1], fshader[1], 1);
	initShader("../shaders_P3/shader.v3.vert", "../shaders_P3/shader.v3.frag", program[2], vshader[2], fshader[2], 2);

	initObj(0);
	initObj(1);
	initObj(2);
	

	glutMainLoop();

	destroy();

	return 0;
}
	
//////////////////////////////////////////
// Funciones auxiliares 
void initContext(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(800, 800);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Prácticas OGL");

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		exit(-1);
	}
	const GLubyte* oglVersion = glGetString(GL_VERSION);
	std::cout << "This system supports OpenGL Version: " << oglVersion << std::endl;

	glutReshapeFunc(resizeFunc);
	glutDisplayFunc(renderFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);
}


void initOGL()
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);


	//Inicializamos las variables de nuestra escena
	proj = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 50.0f);
	view = glm::mat4(1.0f);
	view[3].z = -15;

	model[0] = glm::mat4(1.0f);
	model[1] = glm::mat4(1.0f);
	model[2] = glm::mat4(1.0f);

	lightPos = glm::vec4(-4.0f, 0.0f, 0.0f, 1.0f);
	
	model[2][3].x = lightPos.x;
	model[2][3].y = lightPos.y;
	model[2][3].z = lightPos.z;

	lightAmb = glm::vec3(0.3f);
	lightDif = glm::vec3(1.0f);
	lightSpec = glm::vec3(1.0f);

	colorTexId1 = loadTex("../img/color2.png");
	colorTexId2 = loadTex("../img/gioconda.png");
	emiTexId = loadTex("../img/emissive.png");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorTexId1);

	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, colorTexId2);

	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, emiTexId);
}


void destroy()
{
	for (int i = 0; i < 3; ++i)
	{
		glDetachShader(program[i], vshader[i]);
		glDetachShader(program[i], fshader[i]);
		glDeleteShader(vshader[i]);
		glDeleteShader(fshader[i]);
		glDeleteProgram(program[i]);

		if (attributes[i].inPos != -1) glDeleteBuffers(1, &vbos[i].posVBO);
		if (attributes[i].inColor != -1) glDeleteBuffers(1, &vbos[i].colorVBO);
		if (attributes[i].inNormal != -1) glDeleteBuffers(1, &vbos[i].normalVBO);
		if (attributes[i].inTexCoord != -1) glDeleteBuffers(1, &vbos[i].texCoordVBO);

		glDeleteBuffers(1, &vbos[i].triangleIndexVBO);
		glDeleteVertexArrays(1, &vao[i]);
	}

	glDeleteTextures(1, &colorTexId1);
	glDeleteTextures(1, &colorTexId2);
	glDeleteTextures(1, &emiTexId);
}

void initShader(const char *vname, const char *fname, unsigned int & program, unsigned int & vshader, unsigned int & fshader, size_t i)
{
	vshader = loadShader(vname, GL_VERTEX_SHADER);
	fshader = loadShader(fname, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glLinkProgram(program);

	int linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetProgramInfoLog(program, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteProgram(program);
		program = 0;
		exit(-1);
	}

	glBindAttribLocation(program, 0, "inPos");
	glBindAttribLocation(program, 1, "inColor");
	glBindAttribLocation(program, 2, "inNormal");
	glBindAttribLocation(program, 3, "inTexCoord");

	attributes[i].inPos = glGetAttribLocation(program, "inPos");
	attributes[i].inColor = glGetAttribLocation(program, "inColor");
	attributes[i].inNormal = glGetAttribLocation(program, "inNormal");
	attributes[i].inTexCoord = glGetAttribLocation(program, "inTexCoord");

	uniforms[i].uNormalMat = glGetUniformLocation(program, "normal");
	uniforms[i].uModelViewMat = glGetUniformLocation(program, "modelView");
	uniforms[i].uModelViewProjMat = glGetUniformLocation(program, "modelViewProj");
	uniforms[i].uColorTex = glGetUniformLocation(program, "colorTex");
	uniforms[i].uEmiTex = glGetUniformLocation(program, "emiTex");
	uniforms[i].uLightPos = glGetUniformLocation(program, "lightPos");
	uniforms[i].uLightAmb = glGetUniformLocation(program, "Ia");
	uniforms[i].uLightDif = glGetUniformLocation(program, "Id");
	uniforms[i].uLightSpec = glGetUniformLocation(program, "Is");
}


void initObj(size_t i)
{
	glGenVertexArrays(1, &vao[i]);
	glBindVertexArray(vao[i]);

	if (attributes[i].inPos != -1)
	{
		glGenBuffers(1, &vbos[i].posVBO);
		glBindBuffer(GL_ARRAY_BUFFER, vbos[i].posVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3, cubeVertexPos, GL_STATIC_DRAW);
		glVertexAttribPointer(attributes[i].inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(attributes[i].inPos);
	}
	if (attributes[i].inColor != -1)
	{
		glGenBuffers(1, &vbos[i].colorVBO);
		glBindBuffer(GL_ARRAY_BUFFER, vbos[i].colorVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3, cubeVertexColor, GL_STATIC_DRAW);
		glVertexAttribPointer(attributes[i].inColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(attributes[i].inColor);
	}
	if (attributes[i].inNormal != -1)
	{
		glGenBuffers(1, &vbos[i].normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, vbos[i].normalVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3, cubeVertexNormal, GL_STATIC_DRAW);
		glVertexAttribPointer(attributes[i].inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(attributes[i].inNormal);
	}
	if (attributes[i].inTexCoord != -1)
	{
		glGenBuffers(1, &vbos[i].texCoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, vbos[i].texCoordVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 2, cubeVertexTexCoord, GL_STATIC_DRAW);
		glVertexAttribPointer(attributes[i].inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(attributes[i].inTexCoord);
	}

	glGenBuffers(1, &vbos[i].triangleIndexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[i].triangleIndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeNTriangleIndex * sizeof(unsigned int) * 3, cubeTriangleIndex, GL_STATIC_DRAW);
}


GLuint loadShader(const char *fileName, GLenum type)
{
	unsigned int fileLen;
	char* source = loadStringFromFile(fileName, fileLen);
	//////////////////////////////////////////////
	//Creación y compilación del Shader
	GLuint shader;
	shader = glCreateShader(type);
	glShaderSource(shader, 1, (const GLchar * *)& source, (const GLint*)& fileLen);
	glCompileShader(shader);
	delete[] source;

	//Comprobamos que se compiló bien
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		std::cout << fileName << std::endl;
		glGetShaderInfoLog(shader, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteShader(shader);
		exit(-1);
	}
	return shader; 
}


unsigned int loadTex(const char *fileName)
{
	unsigned char* map;
	unsigned int w, h;
	map = loadTexture(fileName, w, h);
	if (!map)
	{
		std::cout << "Error cargando el fichero: "
			<< fileName << std::endl;
		exit(-1);
	}

	unsigned int texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)map);

	delete[] map;

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);

	return texId;
}


void renderFunc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::vec4 lightViewPos = view * lightPos;

	for (int i = 0; i < 3; ++i)
	{
		glUseProgram(program[i]);

		glm::mat4 modelView = view * model[i];
		glm::mat4 modelViewProj = proj * view * model[i];
		glm::mat4 normal = glm::transpose(glm::inverse(modelView));

		//Matrices uniform
		if (uniforms[i].uModelViewMat != -1)
			glUniformMatrix4fv(uniforms[i].uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));
		if (uniforms[i].uModelViewProjMat != -1)
			glUniformMatrix4fv(uniforms[i].uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));
		if (uniforms[i].uNormalMat != -1)
			glUniformMatrix4fv(uniforms[i].uNormalMat, 1, GL_FALSE, &(normal[0][0]));

		//Texturas uniform
		if (uniforms[i].uColorTex != -1)
			glUniform1i(uniforms[0].uColorTex, i);
		if (uniforms[i].uEmiTex != -1)
			glUniform1i(uniforms[i].uEmiTex, 2);

		//Otros uniform
		if(uniforms[i].uLightPos != -1)
			glUniform4fv(uniforms[i].uLightPos, 1, &lightViewPos[0]);
		if (uniforms[i].uLightAmb != -1)
			glUniform3fv(uniforms[i].uLightAmb, 1, &lightAmb[0]);
		if (uniforms[i].uLightDif != -1)
			glUniform3fv(uniforms[i].uLightDif, 1, &lightDif[0]);
		if (uniforms[i].uLightSpec != -1)
			glUniform3fv(uniforms[i].uLightSpec, 1, &lightSpec[0]);

		glBindVertexArray(vao[i]);
		glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void*)0);
	}

	glutSwapBuffers();
}


void resizeFunc(int width, int height)
{
	glViewport(0, 0, width, height);
	float aspectRatio = float(width) / float(height);
	proj = glm::perspective(glm::radians(60.0f), aspectRatio, 0.1f, 50.0f);
	glutPostRedisplay();
}


void idleFunc()
{
	static float angle = 0.0f;
	angle = (angle > 3.141592f * 2.0f) ? 0 : angle + 0.01f;
	float radius = 3.0f;
	float x = radius * glm::cos(angle);
	float y = radius * glm::sin(angle);

	model[0] = glm::mat4(1.0f);
	model[0] = glm::rotate(model[0], angle, glm::vec3(1.0f, 1.0f, 0.0f));

	model[1] = glm::mat4(1.0f);
	model[1] = glm::translate(model[1], glm::vec3(x, y, x));
	model[1] = glm::rotate(model[1], angle, glm::vec3(0.0f, 1.0f, 0.0f));

	glutPostRedisplay();
}


void keyboardFunc(unsigned char key, int x, int y)
{
	float lightStep = 0.2;
	switch (key)
	{
	case('A'):
	case('a'):
		lightPos.x -= lightStep;
		model[2][3].x = lightPos.x;
		break;

	case('D'):
	case('d'):
		lightPos.x += lightStep;
		model[2][3].x = lightPos.x;
		break;

	case('W'):
	case('w'):
		lightPos.z -= lightStep;
		model[2][3].z = lightPos.z;
		break;

	case('S'):
	case('s'):
		lightPos.z += lightStep;
		model[2][3].z = lightPos.z;
		break;

	case('1'):
		lightAmb = glm::min(lightAmb+glm::vec3(0.1f), glm::vec3(1.0f));
		break;

	case('2'):
		lightAmb = glm::max(lightAmb-glm::vec3(0.1f), glm::vec3(0.0f));
		break;

	case('3'):
		lightDif = glm::min(lightDif+glm::vec3(0.1f), glm::vec3(1.0f));
		break;

	case('4'):
		lightDif = glm::max(lightDif-glm::vec3(0.1f), glm::vec3(0.0f));
		break;

	case('5'):
		lightSpec = glm::min(lightSpec+glm::vec3(0.1f), glm::vec3(1.0f));
		break;

	case('6'):
		lightSpec = glm::max(lightSpec-glm::vec3(0.1f), glm::vec3(0.0f));
		break;
	}


}


void mouseFunc(int button, int state, int x, int y){}









