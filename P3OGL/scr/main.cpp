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
glm::mat4	modelCube1 = glm::mat4(1.0f);
glm::mat4	modelCube2 = glm::mat4(1.0f);
glm::vec4	lightPos = glm::vec4(0.0f);
glm::vec3	lightAmb = glm::vec3(0.3f);
glm::vec3	lightDif = glm::vec3(1.0f);
glm::vec3	lightSpec = glm::vec3(1.0f);



//////////////////////////////////////////////////////////////
// Variables que nos dan acceso a Objetos OpenGL
//////////////////////////////////////////////////////////////
//Por definir
unsigned int vshader;
unsigned int fshader;
unsigned int program;

//Variables Uniform
int uModelViewMat;
int uModelViewProjMat;
int uNormalMat;
int uColorTex;
int uEmiTex;
int uLightPos;
int uLightAmb;
int uLightDif;
int uLightSpec;

//Texturas
unsigned int colorTexId;
unsigned int emiTexId;

//Atributos
int inPos;
int inColor;
int inNormal;
int inTexCoord;

//VAO
unsigned int vao;
//VBOs que forman parte del objeto
unsigned int posVBO;
unsigned int colorVBO;
unsigned int normalVBO;
unsigned int texCoordVBO;
unsigned int triangleIndexVBO;



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
void initShader(const char *vname, const char *fname);
void initObj();
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
	initShader("../shaders_P3/shader.v1.vert", "../shaders_P3/shader.v1.frag");
	initObj();

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
	glutInitWindowSize(500, 500);
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

	proj = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 50.0f);
	view = glm::mat4(1.0f);
	view[3].z = -15;
}


void destroy()
{
	glDetachShader(program, vshader);
	glDetachShader(program, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	glDeleteProgram(program);

	if (inPos != -1) glDeleteBuffers(1, &posVBO);
	if (inColor != -1) glDeleteBuffers(1, &colorVBO);
	if (inNormal != -1) glDeleteBuffers(1, &normalVBO);
	if (inTexCoord != -1) glDeleteBuffers(1, &texCoordVBO);
	glDeleteTextures(1, &colorTexId);
	glDeleteTextures(1, &emiTexId);
	glDeleteBuffers(1, &triangleIndexVBO);
	glDeleteVertexArrays(1, &vao);

}

void initShader(const char *vname, const char *fname)
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

	inPos = glGetAttribLocation(program, "inPos");
	inColor = glGetAttribLocation(program, "inColor");
	inNormal = glGetAttribLocation(program, "inNormal");
	inTexCoord = glGetAttribLocation(program, "inTexCoord");

	uNormalMat = glGetUniformLocation(program, "normal");
	uModelViewMat = glGetUniformLocation(program, "modelView");
	uModelViewProjMat = glGetUniformLocation(program, "modelViewProj");
	uColorTex = glGetUniformLocation(program, "colorTex");
	uEmiTex = glGetUniformLocation(program, "emiTex");
	uLightPos = glGetUniformLocation(program, "lightPos");
	uLightAmb = glGetUniformLocation(program, "Ia");
	uLightDif = glGetUniformLocation(program, "Id");
	uLightSpec = glGetUniformLocation(program, "Is");
}


void initObj()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	if (inPos != -1)
	{
		glGenBuffers(1, &posVBO);
		glBindBuffer(GL_ARRAY_BUFFER, posVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3, cubeVertexPos, GL_STATIC_DRAW);
		glVertexAttribPointer(inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inPos);
	}
	if (inColor != -1)
	{
		glGenBuffers(1, &colorVBO);
		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3, cubeVertexColor, GL_STATIC_DRAW);
		glVertexAttribPointer(inColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inColor);
	}
	if (inNormal != -1)
	{
		glGenBuffers(1, &normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3, cubeVertexNormal, GL_STATIC_DRAW);
		glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inNormal);
	}
	if (inTexCoord != -1)
	{
		glGenBuffers(1, &texCoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 2, cubeVertexTexCoord, GL_STATIC_DRAW);
		glVertexAttribPointer(inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inTexCoord);
	}

	glGenBuffers(1, &triangleIndexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeNTriangleIndex * sizeof(unsigned int) * 3, cubeTriangleIndex, GL_STATIC_DRAW);


	colorTexId = loadTex("../img/color2.png");
	emiTexId = loadTex("../img/emissive.png");

	modelCube1 = glm::mat4(1.0f);
	modelCube2 = glm::mat4(1.0f);
	lightPos = glm::vec4(-4.0f, 0.0f, 0.0f, 1.0f);

	lightAmb = glm::vec3(0.3f);
	lightDif = glm::vec3(1.0f);
	lightSpec = glm::vec3(1.0f);
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
	glUseProgram(program);
	//? pintado del objeto!!!!

	//Texturas
	if (uColorTex != -1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorTexId);
		glUniform1i(uColorTex, 0);
	}
	if (uEmiTex != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, emiTexId);
		glUniform1i(uEmiTex, 1);
	}

	//Dibujado del primer cubo
	glm::mat4 modelView = view * modelCube1;
	glm::mat4 modelViewProj = proj * view * modelCube1;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));

	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE,	&(normal[0][0]));

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void*)0);

	//Dibujado del segundo cubo
	modelCube2[3].x = 4.0f;
	modelView = view * modelCube2;
	modelViewProj = proj * modelView;
	normal = glm::transpose(glm::inverse(modelView));

	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE, &(normal[0][0]));

	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void*)0);

	//Luz
	lightPos.w = 1;
	glm::vec4 light = view * lightPos;
	glUniform4fv(uLightPos, 1, &light[0]);
	glUniform3fv(uLightAmb, 1, &lightAmb[0]);
	glUniform3fv(uLightDif, 1, &lightDif[0]);
	glUniform3fv(uLightSpec, 1, &lightSpec[0]);

	//Dibujado de un tercer cubo con la posicion de la luz
	glm::mat4 modelLight = glm::mat4(1.0);
	modelLight[3].x = lightPos.x;
	modelLight[3].y = lightPos.y;
	modelLight[3].z = lightPos.z;
	modelView = view * modelLight;
	modelViewProj = proj * view * modelLight;
	normal = glm::transpose(glm::inverse(modelLight));

	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE, &(modelLight[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE, &(normal[0][0]));
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void*)0);

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
	modelCube1 = glm::mat4(1.0f);
	static float angle = 0.0f;
	angle = (angle > 3.141592f * 2.0f) ? 0 : angle + 0.01f;
	modelCube1 = glm::rotate(modelCube1, angle, glm::vec3(1.0f, 1.0f, 0.0f));

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
		break;

	case('D'):
	case('d'):
		lightPos.x += lightStep;
		break;

	case('W'):
	case('w'):
		lightPos.z -= lightStep;
		break;

	case('S'):
	case('s'):
		lightPos.z += lightStep;
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









