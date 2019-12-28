#ifndef __PYRAMIDFILE__
#define __PYRAMIDFILE__

#include <glm/glm.hpp>

//Describimos una piramide
//*******************
//Se replican vértices para que cada cara 
//tenga una normal distinta.

class Pyramid {
	public:
		const int nVertex = 24; // 3 vértices x 8 caras
		const int nTriangleIndex = 8; // 1 triángulo por cara x 8 cara;
		const unsigned int triangleIndex[24] = {
			//Cara 1 sup
			0,1,2,
			//Cara 2 sup
			3,4,5,
			//Cara 3 sup
			6,7,8,
			//Cara 4 sup
			9,10,11,
			//Cara 1 inf
			12,13,14,
			//Cara 2 inf
			15,16,17,
			//Cara 3 inf
			18,19,20,
			//Cara 4 inf
			21,22,23,
		};

		//Posicíon de los vertices
		const float vertexPos[72] = {
			//Cara 1 sup
			 1.0f,	 0.0f,	-1.0f, //0
			 1.0f,	 0.0f,	 1.0f, //1
			 0.0f,	 1.0f,	 0.0f, //2

			//Cara 2 sup		   
			 1.0f,	 0.0f,	 1.0f, //3
			-1.0f,	 0.0f,	 1.0f, //4
			 0.0f,	 1.0f,	 0.0f, //5

			//Cara 3 sup		   
		   -1.0f,	 0.0f,	 1.0f, //6
		   -1.0f,	 0.0f,	-1.0f, //7
			0.0f,	 1.0f,	 0.0f, //8

			//Cara 4 sup		   
			-1.0f,	 0.0f,	-1.0f, //9
			 1.0f,	 0.0f,	-1.0f, //10
			 0.0f,	 1.0f,	 0.0f, //11

			 //Cara 1 inf
			 1.0f,	 0.0f,	-1.0f, //12
			 0.0f,	-1.0f,	 0.0f, //13
			 1.0f,	 0.0f,	 1.0f, //14

			//Cara 2 inf		   
			 1.0f,	 0.0f,	 1.0f, //15
			 0.0f,	-1.0f,	 0.0f, //16
			-1.0f,	 0.0f,	 1.0f, //17


			//Cara 3 inf		   
		   -1.0f,	 0.0f,	 1.0f, //18
			0.0f,	-1.0f,	 0.0f, //19
		   -1.0f,	 0.0f,	-1.0f, //20


			//Cara 4 inf		   
			-1.0f,	 0.0f,	-1.0f, //21
			 0.0f,	-1.0f,	 0.0f, //22
			 1.0f,	 0.0f,	-1.0f, //23
		};


		//Color de los vertices
		const float vertexColor[72] = {
			//Cara 1 sup
			1.0f,	0.0f,	 0.0f,
			1.0f,	0.0f,	 0.0f,
			1.0f,	0.0f,	 0.0f,

			//Cara 2 sup		   
			0.0f,	1.0f,	 0.0f,
			0.0f,	1.0f,	 0.0f,
			0.0f,	1.0f,	 0.0f,

			//Cara 3 sup	   
			1.0f,	0.0f,	 0.0f,
			1.0f,	0.0f,	 0.0f,
			1.0f,	0.0f,	 0.0f,

			//Cara 4 sup		   
			0.0f,	0.0f,	 1.0f,
			0.0f,	0.0f,	 1.0f,
			0.0f,	0.0f,	 1.0f,

			//Cara 1 inf
			0.0f,	1.0f,	 0.0f,
			0.0f,	1.0f,	 0.0f,
			0.0f,	1.0f,	 0.0f,

			//Cara 2 inf		   
			1.0f,	0.0f,	 0.0f,
			1.0f,	0.0f,	 0.0f,
			1.0f,	0.0f,	 0.0f,

			//Cara 3 inf	   
			0.0f,	0.0f,	 1.0f,
			0.0f,	0.0f,	 1.0f,
			0.0f,	0.0f,	 1.0f,

			//Cara 4 inf		   
			1.0f,	0.0f,	 0.0f,
			1.0f,	0.0f,	 0.0f,
			1.0f,	0.0f,	 0.0f,
		};

		//Cood. de textura de los vertices
		const float vertexTexCoord[48] = {
			//Cara 1 sup
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f, 1.0f,

			//Cara 2 sup
			1.0f, 0.0f,
			0.0f, 0.0f,
			0.0f, 1.0f,

			//Cara 3 sup	
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f, 1.0f,

			//Cara 4 sup
			1.0f, 0.0f,
			0.0f, 0.0f,
			0.0f, 1.0f,

			//Cara 1 inf
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 0.0f,

			//Cara 2 inf
			1.0f, 0.0f,
			0.0f, 1.0f,
			0.0f, 0.0f,

			//Cara 3 inf	
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 0.0f,

			//Cara 4 inf
			1.0f, 0.0f,
			0.0f, 1.0f,
			0.0f, 0.0f,


		};

		float vertexNormal[72];
		float vertexTangent[72];

		Pyramid()
		{
			for (int i = 0; i < nVertex * 3; i = i + 9)
			{
				glm::vec3 a = glm::vec3(vertexPos[i + 0], vertexPos[i + 1], vertexPos[i + 2]);
				glm::vec3 b = glm::vec3(vertexPos[i + 3], vertexPos[i + 4], vertexPos[i + 5]);
				glm::vec3 c = glm::vec3(vertexPos[i + 6], vertexPos[i + 7], vertexPos[i + 8]);

				glm::vec3 ab = glm::normalize(b - a);
				glm::vec3 ac = glm::normalize(c - a);
				glm::vec3 N = glm::normalize(glm::cross(ab, ac));

				vertexNormal[i + 0] = N.x;
				vertexNormal[i + 1] = N.y;
				vertexNormal[i + 2] = N.z;
				vertexNormal[i + 3] = N.x;
				vertexNormal[i + 4] = N.y;
				vertexNormal[i + 5] = N.z;
				vertexNormal[i + 6] = N.x;
				vertexNormal[i + 7] = N.y;
				vertexNormal[i + 8] = N.z;

				vertexTangent[i + 0] = ab.x;
				vertexTangent[i + 1] = ab.y;
				vertexTangent[i + 2] = ab.z;
				vertexTangent[i + 3] = ab.x;
				vertexTangent[i + 4] = ab.y;
				vertexTangent[i + 5] = ab.z;
				vertexTangent[i + 6] = ab.x;
				vertexTangent[i + 7] = ab.y;
				vertexTangent[i + 8] = ab.z;
			}
		}	

};

#endif

