#include "BOX.h"
#include "PYRAMID.hpp"
#include "auxiliar.h"
#include <vector>

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
glm::mat4	model[5];

//Otras variables de la luz
//Propeidades de la fuente de luz focal
struct SpotLight {
	glm::vec3 intensity;
	glm::vec3 position;
	glm::vec3 direction;

	float constant;
	float linear;
	float quadratic;

	float cutOff;
	float m;
};
SpotLight spotLight;

//Propiedades de luz puntual
glm::vec4	lightPos;
glm::vec3	lightAmb;
glm::vec3	lightDif;
glm::vec3	lightSpec;

// Variables de control de la cámara
glm::vec3 cameraPos, cameraForward, cameraUp;

// Control del movimiento del ratón
float x_pressed, y_pressed;

// Puntos de control de trayectoria
std::vector <glm::vec3> controlPoints;
bool firstCurve = true;
bool firstCurve2 = true;

Pyramid pyramid;

//////////////////////////////////////////////////////////////
// Variables que nos dan acceso a Objetos OpenGL
//////////////////////////////////////////////////////////////
//Por definir
unsigned int vshader[4];
unsigned int fshader[4];
unsigned int program[4];

//Variables Uniform
struct UniformsSpotLight {
	int intensity;
	int position;
	int direction;
	int constant;
	int linear;
	int quadratic;
	int cutOff;
	int m;
};

struct Uniforms {
	int uModelViewMat;
	int uModelViewProjMat;
	int uNormalMat;
	int uColorTex;
	int uEmiTex;
	int uNormalTex;
	int uSpecTex;
	int uLightPos;
	int uLightAmb;
	int uLightDif;
	int uLightSpec;
	UniformsSpotLight uSpotLight;
};
Uniforms uniforms[4];

//Texturas
unsigned int colorTexId1, colorTexId2, emiTexId, normalTexId, specularTexId;

//Atributos
struct Attributes {
	int inPos;
	int inColor;
	int inNormal;
	int inTangent;
	int inTexCoord;
};
Attributes attributes[4];

//VAO
unsigned int vao[5];
//VBOs que forman parte del objeto
struct VBOS {
	unsigned int posVBO;
	unsigned int colorVBO;
	unsigned int normalVBO;
	unsigned int tangentVBO;
	unsigned int texCoordVBO;
	unsigned int triangleIndexVBO;
};
VBOS vbos[5];

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
void mouseMotionFunc(int x, int y);

//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initShader(const char* vname, const char* fname, unsigned int&, unsigned int&, unsigned int&, size_t);
void initObj(size_t, const float*, const float*, const float*, const float*, const float*, const unsigned int*, int, int);
void destroy();

//Carga el shader indicado, devuele el ID del shader
//!Por implementar
GLuint loadShader(const char* fileName, GLenum type);

//Crea una textura, la configura, la sube a OpenGL,
//y devuelve el identificador de la textura
//!!Por implementar
unsigned int loadTex(const char* fileName);

int main(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));// acentos ;)

	initContext(argc, argv);
	initOGL();

	//Inicialización de los 4 pares de shaders (los dos ultimos objetos comparten el mismo shader)
	initShader("../shaders_P3/shader.v1.vert", "../shaders_P3/shader.v1.frag", program[0], vshader[0], fshader[0], 0);
	initShader("../shaders_P3/shader.v2.vert", "../shaders_P3/shader.v2.frag", program[1], vshader[1], fshader[1], 1);
	initShader("../shaders_P3/shader.v3.vert", "../shaders_P3/shader.v3.frag", program[2], vshader[2], fshader[2], 2);
	initShader("../shaders_P3/shader.v4.vert", "../shaders_P3/shader.v4.frag", program[3], vshader[3], fshader[3], 3);

	//Inicialización de 4 cubos y un octaedro
	initObj(0, cubeVertexPos, cubeVertexColor, cubeVertexNormal, cubeVertexTangent, cubeVertexTexCoord, cubeTriangleIndex, cubeNVertex, cubeNTriangleIndex);
	initObj(1, cubeVertexPos, cubeVertexColor, cubeVertexNormal, cubeVertexTangent, cubeVertexTexCoord, cubeTriangleIndex, cubeNVertex, cubeNTriangleIndex);
	initObj(2, cubeVertexPos, cubeVertexColor, cubeVertexNormal, cubeVertexTangent, cubeVertexTexCoord, cubeTriangleIndex, cubeNVertex, cubeNTriangleIndex);
	initObj(3, cubeVertexPos, cubeVertexColor, cubeVertexNormal, cubeVertexTangent, cubeVertexTexCoord, cubeTriangleIndex, cubeNVertex, cubeNTriangleIndex);

	//OPCIONAL 2.c
	//CREACIÓN DE UN NUEVO MODELO CON SUS NORMALES Y TANGENTES CALCULADAS
	Pyramid pyramid = Pyramid();
	initObj(4, pyramid.vertexPos, pyramid.vertexColor, pyramid.vertexNormal, pyramid.vertexTangent, pyramid.vertexTexCoord, pyramid.triangleIndex, pyramid.nVertex, pyramid.nTriangleIndex);

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
	glutMotionFunc(mouseMotionFunc);
	//glutPassiveMotionFunc(mouseMotionFunc);
}

void initOGL()
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);

	//Inicializamos las variables de nuestra escena
	//Matriz de proyección
	proj = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 50.0f);

	//Matriz de vista
	cameraPos = glm::vec3(0.0f, 0.0f, 20.0f);
	cameraForward = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	view = glm::lookAt(cameraPos, cameraPos + cameraForward, cameraUp);

	//Matrices de nuestros modelos
	model[0] = glm::mat4(1.0f);
	model[1] = glm::mat4(1.0f);
	model[2] = glm::mat4(1.0f);
	model[3] = glm::mat4(1.0f);
	model[4] = glm::mat4(1.0f);

	//El tercer cubo indica la posición de la luz
	lightPos = glm::vec4(-4.0f, 0.0f, 0.0f, 1.0f);
	model[2][3].x = lightPos.x;
	model[2][3].y = lightPos.y;
	model[2][3].z = lightPos.z;

	//OPCIONAL 2.d
	//ILUMINACIÓN DE LA ESCENA CON LUCES DE DISTINTO TIPO (PUNTUAL Y FOCAL)
	//La luz focal siempre está ubicada donde la cámara y apuntando hacia donde apunta la misma
	spotLight = SpotLight();
	spotLight.intensity = glm::vec3(1.0f);
	spotLight.position = glm::vec3(0.0f, 0.0f, 0.0f);
	spotLight.direction = glm::vec3(0, 0, -1);
	spotLight.constant = 1;
	spotLight.linear = 0;
	spotLight.quadratic = 0;
	spotLight.cutOff = 0.97;
	spotLight.m = 2;

	//Propiedades de la luz puntual
	lightAmb = glm::vec3(0.15f);
	lightDif = glm::vec3(0.4f);
	lightSpec = glm::vec3(1.0f);

	// Creacion de puntos de control para la órbita de los dos últimos objetos
	glm::vec3 points[] = {
		glm::vec3(-4.5f,0.0f,0.0f), //P0
		glm::vec3(-1.5f,8.0f,10.0f), //P1
		glm::vec3(4.5f,8.0f,10.0f), //P2
		glm::vec3(4.5f,0.0f,0.0f), //P3
		glm::vec3(4.5f,-8.0f,-10.0f), //P4
		glm::vec3(-1.5f,-8.0f,-10.0f), //P5
	};
	controlPoints.insert(controlPoints.begin(), points, points + 6);

	//Lectura y carga de texturas
	colorTexId1 = loadTex("../img/color2.png");
	colorTexId2 = loadTex("../img/gioconda.png");
	emiTexId = loadTex("../img/emissive.png");
	normalTexId = loadTex("../img/normal.png");
	specularTexId = loadTex("../img/specMap.png");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorTexId1);

	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, colorTexId2);

	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, emiTexId);

	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, normalTexId);

	//OPCIONAL 2.g
	//USO DE TEXTURA ESPECULAR
	glActiveTexture(GL_TEXTURE0 + 4);
	glBindTexture(GL_TEXTURE_2D, specularTexId);
}

void destroy()
{
	for (int i = 0; i < 4; ++i)
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
	glDeleteTextures(1, &normalTexId);
	glDeleteTextures(1, &specularTexId);
}

void initShader(const char* vname, const char* fname, unsigned int& program, unsigned int& vshader, unsigned int& fshader, size_t i)
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
	glBindAttribLocation(program, 3, "inTangent");
	glBindAttribLocation(program, 4, "inTexCoord");

	//Almacenamos los identificadores de atributos de todos los shaders
	attributes[i].inPos = glGetAttribLocation(program, "inPos");
	attributes[i].inColor = glGetAttribLocation(program, "inColor");
	attributes[i].inNormal = glGetAttribLocation(program, "inNormal");
	attributes[i].inTangent = glGetAttribLocation(program, "inTangent");
	attributes[i].inTexCoord = glGetAttribLocation(program, "inTexCoord");

	//Almacenamos los identificadores de uniforms de todos los shaders
	uniforms[i].uNormalMat = glGetUniformLocation(program, "normal");
	uniforms[i].uModelViewMat = glGetUniformLocation(program, "modelView");
	uniforms[i].uModelViewProjMat = glGetUniformLocation(program, "modelViewProj");
	uniforms[i].uColorTex = glGetUniformLocation(program, "colorTex");
	uniforms[i].uEmiTex = glGetUniformLocation(program, "emiTex");
	uniforms[i].uNormalTex = glGetUniformLocation(program, "normalTex");
	uniforms[i].uSpecTex = glGetUniformLocation(program, "specularTex");
	uniforms[i].uLightPos = glGetUniformLocation(program, "lightPos");
	uniforms[i].uLightAmb = glGetUniformLocation(program, "Ia");
	uniforms[i].uLightDif = glGetUniformLocation(program, "Id");
	uniforms[i].uLightSpec = glGetUniformLocation(program, "Is");

	uniforms[i].uSpotLight.intensity = glGetUniformLocation(program, "spotLight.intensity");
	uniforms[i].uSpotLight.position = glGetUniformLocation(program, "spotLight.position");
	uniforms[i].uSpotLight.direction = glGetUniformLocation(program, "spotLight.direction");
	uniforms[i].uSpotLight.constant = glGetUniformLocation(program, "spotLight.constant");
	uniforms[i].uSpotLight.linear = glGetUniformLocation(program, "spotLight.linear");
	uniforms[i].uSpotLight.quadratic = glGetUniformLocation(program, "spotLight.quadratic");
	uniforms[i].uSpotLight.cutOff = glGetUniformLocation(program, "spotLight.cutOff");
	uniforms[i].uSpotLight.m = glGetUniformLocation(program, "spotLight.m");
}

void initObj(size_t i, const float* vertexPos, const float* vertexColor, const float* vertexNormal, const float* vertexTangent, const float* vertexTexCoord, const unsigned int* triangleIndex, int nVertex, int nTriangleIndex)
{
	//Usamos diferentes VAOs y VBOs debido a que no todos utilizan todos los atributos (dependen de su shader)
	glGenVertexArrays(1, &vao[i]);
	glBindVertexArray(vao[i]);

	//Con esto nos aseguramos que los dos últimos VAOs utilizan el mismo shader[3]
	int j = (i > 3) ? 3 : i;

	if (attributes[j].inPos != -1)
	{
		glGenBuffers(1, &vbos[i].posVBO);
		glBindBuffer(GL_ARRAY_BUFFER, vbos[i].posVBO);
		glBufferData(GL_ARRAY_BUFFER, nVertex * sizeof(float) * 3, vertexPos, GL_STATIC_DRAW);
		glVertexAttribPointer(attributes[j].inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(attributes[j].inPos);
	}
	if (attributes[j].inColor != -1)
	{
		glGenBuffers(1, &vbos[i].colorVBO);
		glBindBuffer(GL_ARRAY_BUFFER, vbos[i].colorVBO);
		glBufferData(GL_ARRAY_BUFFER, nVertex * sizeof(float) * 3, vertexColor, GL_STATIC_DRAW);
		glVertexAttribPointer(attributes[j].inColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(attributes[j].inColor);
	}
	if (attributes[j].inNormal != -1)
	{
		glGenBuffers(1, &vbos[i].normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, vbos[i].normalVBO);
		glBufferData(GL_ARRAY_BUFFER, nVertex * sizeof(float) * 3, vertexNormal, GL_STATIC_DRAW);
		glVertexAttribPointer(attributes[j].inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(attributes[j].inNormal);
	}
	if (attributes[j].inTangent != -1)
	{
		glGenBuffers(1, &vbos[i].tangentVBO);
		glBindBuffer(GL_ARRAY_BUFFER, vbos[i].tangentVBO);
		glBufferData(GL_ARRAY_BUFFER, nVertex * sizeof(float) * 3, vertexTangent, GL_STATIC_DRAW);
		glVertexAttribPointer(attributes[j].inTangent, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(attributes[j].inTangent);
	}
	if (attributes[j].inTexCoord != -1)
	{
		glGenBuffers(1, &vbos[i].texCoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, vbos[i].texCoordVBO);
		glBufferData(GL_ARRAY_BUFFER, nVertex * sizeof(float) * 2, vertexTexCoord, GL_STATIC_DRAW);
		glVertexAttribPointer(attributes[j].inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(attributes[j].inTexCoord);
	}

	glGenBuffers(1, &vbos[i].triangleIndexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[i].triangleIndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, nTriangleIndex * sizeof(unsigned int) * 3, triangleIndex, GL_STATIC_DRAW);
}

GLuint loadShader(const char* fileName, GLenum type)
{
	unsigned int fileLen;
	char* source = loadStringFromFile(fileName, fileLen);
	//////////////////////////////////////////////
	//Creación y compilación del Shader
	GLuint shader;
	shader = glCreateShader(type);
	glShaderSource(shader, 1, (const GLchar**)&source, (const GLint*)&fileLen);
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

unsigned int loadTex(const char* fileName)
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

	//OPCIONAL 1
	//USO DE FILTRO ANISOTRÓPICO
	bool isAnisotropic = false;
	GLfloat fLargest;
	if (glewIsSupported("GL_EXT_texture_filter_anisotropic"))
	{
		std::cout << "anisotropic filtering activated for texture " << fileName << std::endl;
		isAnisotropic = true;
	}

	if (isAnisotropic)
	{
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
	}

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

	//OBLIGATORIO 4
	//CADA MODELO TIENE SU PROPIA PAREJA DE SHADERS
	//Los tres primeros modelos utilizan cada uno su propio shader
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
			glUniform1i(uniforms[i].uColorTex, i); //El primer modelo usa la primera textura, el segundo la segunda textura y el tercero ninguna
		if (uniforms[i].uEmiTex != -1)
			glUniform1i(uniforms[i].uEmiTex, 2);
		if (uniforms[i].uNormalTex != -1)
			glUniform1i(uniforms[i].uNormalTex, 3);
		if (uniforms[i].uSpecTex != -1)
			glUniform1i(uniforms[i].uSpecTex, 4);

		//Uniforms de la luz puntual
		if (uniforms[i].uLightPos != -1)
			glUniform4fv(uniforms[i].uLightPos, 1, &lightViewPos[0]);
		if (uniforms[i].uLightAmb != -1)
			glUniform3fv(uniforms[i].uLightAmb, 1, &lightAmb[0]);
		if (uniforms[i].uLightDif != -1)
			glUniform3fv(uniforms[i].uLightDif, 1, &lightDif[0]);
		if (uniforms[i].uLightSpec != -1)
			glUniform3fv(uniforms[i].uLightSpec, 1, &lightSpec[0]);

		//Variables uniform de la luz focal
		if (uniforms[i].uSpotLight.intensity != -1)
			glUniform3fv(uniforms[i].uSpotLight.intensity, 1, &spotLight.intensity[0]);
		if (uniforms[i].uSpotLight.position != -1)
			glUniform3fv(uniforms[i].uSpotLight.position, 1, &spotLight.position[0]);
		if (uniforms[i].uSpotLight.direction != -1)
			glUniform3fv(uniforms[i].uSpotLight.direction, 1, &spotLight.direction[0]);
		if (uniforms[i].uSpotLight.constant != -1)
			glUniform1f(uniforms[i].uSpotLight.constant, spotLight.constant);
		if (uniforms[i].uSpotLight.linear != -1)
			glUniform1f(uniforms[i].uSpotLight.linear, spotLight.linear);
		if (uniforms[i].uSpotLight.quadratic != -1)
			glUniform1f(uniforms[i].uSpotLight.quadratic, spotLight.quadratic);
		if (uniforms[i].uSpotLight.cutOff != -1)
			glUniform1f(uniforms[i].uSpotLight.cutOff, spotLight.cutOff);
		if (uniforms[i].uSpotLight.m != -1)
			glUniform1f(uniforms[i].uSpotLight.m, spotLight.m);

		glBindVertexArray(vao[i]);
		glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void*)0);
	}

	//Los dos últimos modelos usan el mismo shader ([3])
	glUseProgram(program[3]);

	//Uniforms comunes a los dos modelos

	//Uniforms de la luz puntual
	if (uniforms[3].uLightPos != -1)
		glUniform4fv(uniforms[3].uLightPos, 1, &lightViewPos[0]);
	if (uniforms[3].uLightAmb != -1)
		glUniform3fv(uniforms[3].uLightAmb, 1, &lightAmb[0]);
	if (uniforms[3].uLightDif != -1)
		glUniform3fv(uniforms[3].uLightDif, 1, &lightDif[0]);
	if (uniforms[3].uLightSpec != -1)
		glUniform3fv(uniforms[3].uLightSpec, 1, &lightSpec[0]);

	//Variables uniform de la luz focal
	if (uniforms[3].uSpotLight.intensity != -1)
		glUniform3fv(uniforms[3].uSpotLight.intensity, 1, &spotLight.intensity[0]);
	if (uniforms[3].uSpotLight.position != -1)
		glUniform3fv(uniforms[3].uSpotLight.position, 1, &spotLight.position[0]);
	if (uniforms[3].uSpotLight.direction != -1)
		glUniform3fv(uniforms[3].uSpotLight.direction, 1, &spotLight.direction[0]);
	if (uniforms[3].uSpotLight.constant != -1)
		glUniform1f(uniforms[3].uSpotLight.constant, spotLight.constant);
	if (uniforms[3].uSpotLight.linear != -1)
		glUniform1f(uniforms[3].uSpotLight.linear, spotLight.linear);
	if (uniforms[3].uSpotLight.quadratic != -1)
		glUniform1f(uniforms[3].uSpotLight.quadratic, spotLight.quadratic);
	if (uniforms[3].uSpotLight.cutOff != -1)
		glUniform1f(uniforms[3].uSpotLight.cutOff, spotLight.cutOff);
	if (uniforms[3].uSpotLight.m != -1)
		glUniform1f(uniforms[3].uSpotLight.m, spotLight.m);

	//Primer modelo
	glm::mat4 modelView = view * model[3];
	glm::mat4 modelViewProj = proj * view * model[3];
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));

	//Matrices uniform propias del primer modelo
	if (uniforms[3].uModelViewMat != -1)
		glUniformMatrix4fv(uniforms[3].uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));
	if (uniforms[3].uModelViewProjMat != -1)
		glUniformMatrix4fv(uniforms[3].uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));
	if (uniforms[3].uNormalMat != -1)
		glUniformMatrix4fv(uniforms[3].uNormalMat, 1, GL_FALSE, &(normal[0][0]));

	glBindVertexArray(vao[3]);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void*)0);

	//Segundo modelo
	modelView = view * model[4];
	modelViewProj = proj * view * model[4];
	normal = glm::transpose(glm::inverse(modelView));

	//Matrices uniform propias del segundo modelo
	if (uniforms[3].uModelViewMat != -1)
		glUniformMatrix4fv(uniforms[3].uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));
	if (uniforms[3].uModelViewProjMat != -1)
		glUniformMatrix4fv(uniforms[3].uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));
	if (uniforms[3].uNormalMat != -1)
		glUniformMatrix4fv(uniforms[3].uNormalMat, 1, GL_FALSE, &(normal[0][0]));

	glBindVertexArray(vao[4]);
	glDrawElements(GL_TRIANGLES, pyramid.nTriangleIndex * 3, GL_UNSIGNED_INT, (void*)0);

	glutSwapBuffers();
}

void resizeFunc(int width, int height)
{
	//OBLIGATORIO 2
	//DEFINICIÓN DE MATRIZ DE PROYECCIÓN MANTENIENDO EL ASPECT RATIO
	glViewport(0, 0, width, height);
	float aspectRatio = float(width) / float(height);
	proj = glm::perspective(glm::radians(60.0f), aspectRatio, 0.1f, 50.0f);
	glutPostRedisplay();
}

void idleFunc()
{
	static float angle = 0.0f;
	angle = (angle > 3.141592f * 2.0f) ? 0 : angle + 0.01f;
	
	//Rotación sobre el propio eje del primer cubo
	model[0] = glm::mat4(1.0f);
	model[0] = glm::rotate(model[0], angle, glm::vec3(1.0f, 1.0f, 0.0f));

	//OBLIGATORIO 3
	//SEGUNDO CUBO ROTANDO ALREDEDOR DEL PRIMERO EN UNA ÓRBITA OBLICUA Y SOBRE SU EJE Y 
	float radius = 3.0f;
	float x = radius * glm::cos(angle);
	float y = radius * glm::sin(angle);
	model[1] = glm::mat4(1.0f);
	model[1] = glm::translate(model[1], glm::vec3(x, y, x));
	model[1] = glm::rotate(model[1], angle, glm::vec3(0.0f, 1.0f, 0.0f));


	//OPCIONAL 2.b
	//EL CUARTO Y QUINTO MODELO ORBITAN UNO DETRÁS DE OTRO ALREDEDOR DEL PRIMERO
	//SIGUIENDO DOS CURVAS DE BEZIER DE 3er GRADO
	static float t1 = 0.2f;
	static float t2 = 0.0f;
	//Trayectoria de  cuarto modelo
	glm::vec3 pos;
	if (t1 < 1.0f)
	{
		t1 += 0.01f;
	}
	else
	{
		t1 = 0.01f;
		firstCurve = !firstCurve;
	}

	if (firstCurve)
	{
		pos = std::pow((1 - t1), 3) * controlPoints[0] + 3 * t1 * std::pow(1 - t1, 2) * controlPoints[1]
			+ 3 * t1 * t1 * (1 - t1) * controlPoints[2] + t1 * t1 * t1 * controlPoints[3];
	}
	else
	{
		pos = std::pow((1 - t1), 3) * controlPoints[3] + 3 * t1 * std::pow(1 - t1, 2) * controlPoints[4]
			+ 3 * t1 * t1 * (1 - t1) * controlPoints[5] + t1 * t1 * t1 * controlPoints[0];
	}
	model[3] = glm::translate(glm::mat4(1.0f), glm::vec3(pos));

	//Trayectoria de quinto modelo
	if (t2 < 1.0f)
	{
		t2 += 0.01f;
	}
	else
	{
		t2 = 0.01f;
		firstCurve2 = !firstCurve2;
	}

	if (firstCurve2)
	{
		pos = std::pow((1 - t2), 3) * controlPoints[0] + 3 * t2 * std::pow(1 - t2, 2) * controlPoints[1]
			+ 3 * t2 * t2 * (1 - t2) * controlPoints[2] + t2 * t2 * t2 * controlPoints[3];
	}
	else
	{
		pos = std::pow((1 - t2), 3) * controlPoints[3] + 3 * t2 * std::pow(1 - t2, 2) * controlPoints[4]
			+ 3 * t2 * t2 * (1 - t2) * controlPoints[5] + t2 * t2 * t2 * controlPoints[0];
	}
	model[4] = glm::translate(glm::mat4(1.0f), glm::vec3(pos));

	glutPostRedisplay();
}

void keyboardFunc(unsigned char key, int x, int y)
{
	float step = 0.2f;
	glm::vec3 left;
	glm::mat4 rotation;
	switch (key)
	{
	//OBLIGATORIO 5
	//CONTROL DE LA CÁMARA CON EL TECLADO
	//Adelante, retroceso y movimientos laterales (A,S,D,W) 
	case('A'):
	case('a'):
		left = glm::normalize(glm::cross(cameraUp, cameraForward));
		cameraPos = cameraPos + left * step;
		view = glm::lookAt(cameraPos, cameraPos + cameraForward, cameraUp);
		break;

	case('D'):
	case('d'):
		left = glm::normalize(glm::cross(cameraUp, cameraForward));
		cameraPos = cameraPos - left * step;
		view = glm::lookAt(cameraPos, cameraPos + cameraForward, cameraUp);
		break;

	case('W'):
	case('w'):
		cameraPos = cameraPos + cameraForward * step;
		view = glm::lookAt(cameraPos, cameraPos + cameraForward, cameraUp);
		break;

	case('S'):
	case('s'):
		cameraPos = cameraPos - cameraForward * step;
		view = glm::lookAt(cameraPos, cameraPos + cameraForward, cameraUp);
		break;

	//Giros (Z,X)
	case('Z'):
	case('z'):
		rotation = glm::rotate(glm::mat4(1.0f), 0.01f, glm::vec3(0, 1, 0));
		cameraForward = glm::normalize(glm::vec3(rotation * glm::vec4(cameraForward, 0.0f)));
		view = glm::lookAt(cameraPos, cameraPos + cameraForward, cameraUp);
		break;

	case('X'):
	case('x'):
		rotation = glm::rotate(glm::mat4(1.0f), -0.01f, glm::vec3(0, 1, 0));
		cameraForward = glm::normalize(glm::vec3(rotation * glm::vec4(cameraForward, 0.0f)));
		view = glm::lookAt(cameraPos, cameraPos + cameraForward, cameraUp);
		break;

	//OBLIGATORIO 1
	//CONTROL DE LA POSICIÓN E INTENSIDADES DE LA LUZ PUNTUAL POR TECLADO
	//Adelante, retroceso y movimientos laterales (J,K,L,I)
	case('J'):
	case('j'):
		lightPos.x -= step;
		model[2][3].x = lightPos.x;
		break;

	case('L'):
	case('l'):
		lightPos.x += step;
		model[2][3].x = lightPos.x;
		break;

	case('I'):
	case('i'):
		lightPos.z -= step;
		model[2][3].z = lightPos.z;
		break;

	case('K'):
	case('k'):
		lightPos.z += step;
		model[2][3].z = lightPos.z;
		break;

	//Intensidad ambiental +,- (1,2)
	case('1'):
		lightAmb = glm::min(lightAmb + glm::vec3(0.1f), glm::vec3(1.0f));
		break;

	case('2'):
		lightAmb = glm::max(lightAmb - glm::vec3(0.1f), glm::vec3(0.0f));
		break;

	//Intensidad difusa +,- (3,4)
	case('3'):
		lightDif = glm::min(lightDif + glm::vec3(0.1f), glm::vec3(1.0f));
		break;

	case('4'):
		lightDif = glm::max(lightDif - glm::vec3(0.1f), glm::vec3(0.0f));
		break;

	//Intensidad especular +,- (5,6)
	case('5'):
		lightSpec = glm::min(lightSpec + glm::vec3(0.1f), glm::vec3(1.0f));
		break;

	case('6'):
		lightSpec = glm::max(lightSpec - glm::vec3(0.1f), glm::vec3(0.0f));
		break;
	}
}

void mouseFunc(int button, int state, int x, int y)
{
	if (state == 0)
	{
		// Cuando se pulsa el botón izquierdo
		if (button == 0)
		{
			x_pressed = x;
			y_pressed = y;
		}
	}
}

void mouseMotionFunc(int x, int y)
{
	//OPCIONAL 2.a
	//CONTROL DE LA CÁMARA CON EL RATÓN AL SER PULSADO
	glm::vec3 left;
	glm::vec4 result;
	glm::mat4 horizontal_rotation, vertical_rotation;

	// Cálculo del ángulo mediante la diferencia de (raton - raton pulsado)
	float scale = 0.001f;
	float angle_x = (x - x_pressed) * scale;
	float angle_y = (y - y_pressed) * scale;

	left = glm::normalize(glm::cross(cameraForward, cameraUp));

	// Obtención de las rotaciones
	horizontal_rotation = glm::rotate(glm::mat4(1.0f), angle_x, glm::vec3(0, 1, 0));
	vertical_rotation = glm::rotate(glm::mat4(1.0f), angle_y, left);

	// Aplicación de las rotaciones a los dos vectores que determinan la orientación de la cámara
	result = horizontal_rotation * vertical_rotation * glm::vec4(cameraForward, 0.0f);
	cameraForward = glm::normalize(glm::vec3(result));

	result = horizontal_rotation * vertical_rotation * glm::vec4(cameraUp, 0.0f);
	cameraUp = glm::normalize(glm::vec3(result));

	// Reseteamos la matriz de vista con la nueva orientación
	view = glm::lookAt(cameraPos, cameraPos + cameraForward, cameraUp);

	x_pressed = x;
	y_pressed = y;
}