/*********************************************************
FILE : main.cpp (csci3260 2018-2019 Final project)
*********************************************************/
/*********************************************************
Student Information
Student1 ID:1155077072
Student1 Name:ZHANG Chongzhi
Student2 ID:1155077009
Student2 Name:ZHANG Xizhe
*********************************************************/

//lighting part referenced from Zhang Xizhe 1155077009 Assignment 2

#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"
#include "Dependencies\glm\glm.hpp"
#include "Dependencies\glm\gtc\matrix_transform.hpp"
#include "Dependencies\glm\gtc\type_ptr.hpp"
#include <iostream>
#include <fstream>
#include <vector>

#include <time.h>
#include <math.h>
using namespace std;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

GLint programID;
GLuint TextureID;

GLuint testPlaneVAO, spaceCraftVAO, earthVAO, ringVAO, wonderStarVAO, rockVAO;
GLuint testPlaneVBO;// for testing.
GLuint ringsVAO[4];

GLuint vertixBO, uvBO, normalBO;//BOs(VBO UVBO NBO) of other objects.
GLsizei drawSize_spaceCraft, drawSize_earth, drawSize_ring, drawSize_wonderStar, drawSize_rock;

GLuint spaceCraftTexture, earthTexture, ringTexture, wonderStarTexture, rockTexture, changedTexture;
bool spaceCraftChangeTexture = false;

GLuint earthNormalTex;

//skybox with cube mapping
GLuint skyboxVAO, skyboxVBO;
GLuint skyboxTex;
vector<const GLchar*> faces;

//transformation control parameters
//skybox size
float skyVertex = 3000.0;
//global self-rotation parameters
float rotationParam = 0.0;
float rotationSpeed = 0.0008;
//ambient light
float amb = 0.5;
//diffuse light
float dif = 1.0;
float lightx = 0.0;
float lightz = 0.0;
//specular light
float spec = 0.5;

//spacecraft movement !!sc for spacecraft
float scForwardPace = 5.0f, scLeftPace = 3.0f, scRotatePace = 0.01f;
int scForwardNum = 0, scLeftNum = 0, scLeftRotateNum = 0;
float scPositionX = 0, scPositionZ = 0;
bool mousePositionIni = false;
int originMouseX;

//Asteroid Ring
GLuint rocksNum = 200;
GLfloat radius = 150.0f;
GLfloat offset = 20.0f;
mat4 * modelMatrices;

// Positions
vec3 * rockXYZ;
vec3 * rockPosition;
vec3 ringPosition[4] = {
	vec3(-50.0f, 0.0f, -800.0f),
	vec3(-60.0f, 0.0f, -1000.0f),
	vec3(20.0f, 0.0f, -1300.0f),
	vec3(-50.0f, 0.0f, -1600.0f)
};
vec3 earthPosition = vec3(0.0f, -40.0f, -2000.0f);
vec3 wonderStarPosition = vec3(-50.0f, -40.0f, -500.0f);
vec3 rockTransformVector = vec3(-50.0f, 0.0f, -500.0f);

int objectDisappear[210] = { 0 };

bool distanceLessThan(int index, float threshold) {
	//index 0-3 ring
	//4 earth
	//5 star
	//6-205 rocks
	if (index <= 3) {
		if (glm::distance(ringPosition[index], vec3(scPositionX, 0.0f, scPositionZ)) < threshold)
			return TRUE;
		else
			return FALSE;
	}
	else if (index == 4) {
		if (glm::distance(earthPosition, vec3(scPositionX, 0.0f, scPositionZ)) < threshold) {
			objectDisappear[4] = 1;
			return true;
		}
		else 
			return false;
	}
	else if (index == 5) {
		if (glm::distance(wonderStarPosition, vec3(scPositionX, 0.0f, scPositionZ)) < threshold) {
			objectDisappear[5] = 1;
			return true;
		}
		else
			return false;
	}
	else if (index > 5) {
		if (glm::distance(rockPosition[index-6], vec3(scPositionX, 0.0f, scPositionZ)) < threshold) {
			objectDisappear[index] = 1;
			return true;
		}
		else
			return false;

	}
	
}

void createModelMatrices() {
	modelMatrices = new mat4[rocksNum];
	rockPosition = new vec3[rocksNum];
	rockXYZ = new vec3[rocksNum];
	srand(glutGet(GLUT_ELAPSED_TIME));
	for (GLuint i = 0; i < rocksNum; i++)
	{
		glm::mat4 model;
		// 1. Translation: displace along circle with 'radius' in range [-offset, offset]
		GLfloat angle = (GLfloat)i / (GLfloat)rocksNum * 360.0f;
		GLfloat displacement = (rand() % (GLint)(2 * offset * 100)) / 100.0f - offset;

		GLfloat x = sin(angle) * radius + displacement;
		displacement = (rand() % (GLint)(2 * offset * 100)) / 100.0f - offset;

		GLfloat y = displacement * 1.0f; // y value has smaller displacement
		displacement = (rand() % (GLint)(2 * offset * 100)) / 100.0f - offset;

		GLfloat z = cos(angle) * radius + displacement;

		model = glm::translate(model, glm::vec3(x, y, z));
		

		// 2. Scale: Scale between 0.05 and 0.25f
		GLfloat scale = (rand() % 20) / 10.0f + 0.5;
		model = glm::scale(model, glm::vec3(scale));

		// 3. Rotation: add random rotation around a (semi)randomly picked rotation axis vector
		GLfloat rotAngle = (rand() % 360);
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		// 4. Now add to list of matrices
		modelMatrices[i] = model;
		rockXYZ[i] = vec3(x, y, z);
	}


}

//a series utilities for setting shader parameters 
void setMat4(const std::string &name, glm::mat4& value)
{
	unsigned int transformLoc = glGetUniformLocation(programID, name.c_str());
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(value));
}

void setVec4(const std::string &name, glm::vec4 value)
{
	glUniform4fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}
void setVec3(const std::string &name, glm::vec3 value)
{
	glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]);
}
void setFloat(const std::string &name, float value)
{
	glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}
void setInt(const std::string &name, int value)
{
	glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}

bool checkStatus(
	GLuint objectID,
	PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
	PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
	GLenum statusType)
{
	GLint status;
	objectPropertyGetterFunc(objectID, statusType, &status);
	if (status != GL_TRUE)
	{
		GLint infoLogLength;
		objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar* buffer = new GLchar[infoLogLength];

		GLsizei bufferSize;
		getInfoLogFunc(objectID, infoLogLength, &bufferSize, buffer);
		cout << buffer << endl;

		delete[] buffer;
		return false;
	}
	return true;
}

bool checkShaderStatus(GLuint shaderID)
{
	return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool checkProgramStatus(GLuint programID)
{
	return checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

string readShaderCode(const char* fileName)
{
	ifstream meInput(fileName);
	if (!meInput.good())
	{
		cout << "File failed to load..." << fileName;
		exit(1);
	}
	return std::string(
		std::istreambuf_iterator<char>(meInput),
		std::istreambuf_iterator<char>()
	);
}

void installShaders()
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar* adapter[1];
	string temp = readShaderCode("VertexShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	temp = readShaderCode("FragmentShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
		return;

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	if (!checkProgramStatus(programID))
		return;

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	glUseProgram(programID);
}

void keyboard(unsigned char key, int x, int y)
{
	if (key == 'q') {
		dif += 0.1;
	}
	if (key == 'w' && dif > 0) {
		dif -= 0.1;
	}
	if (key == 'a') {
		amb += 0.1;
	}
	if (key == 'd' && amb > 0) {
		amb -= 0.1;
	}
	if (key == 'z') {
		spec += 0.1;
	}
	if (key == 'x' && spec > 0) {
		spec -= 0.1;
	}
	if (key == 'i') {
		lightz -= 3.0;
	}
	if (key == 'k') {
		lightz += 3.0;
	}
	if (key == 'j') {
		lightx -= 3.0;
	}
	if (key == 'l') {
		lightx += 3.0;
	}
	
	//TODO: Use keyboard to do interactive events and animation

}

void move(int key, int x, int y) 
{
	//TODO: Use arrow keys to do interactive events and animation
	if (key == GLUT_KEY_UP) {
		scForwardNum++;
		//jeepPosition += jeepForwardPace * jeepDirection;
		scPositionX += scForwardPace * cos(1.57f + scLeftRotateNum * scRotatePace);
		scPositionZ -= scForwardPace * sin(1.57f + scLeftRotateNum * scRotatePace);
	}
	else if (key == GLUT_KEY_DOWN) {
		scForwardNum--;
		//jeepPosition -= jeepForwardPace * jeepDirection;
		scPositionX -= scForwardPace * cos(1.57f + scLeftRotateNum * scRotatePace);
		scPositionZ += scForwardPace * sin(1.57f + scLeftRotateNum * scRotatePace);
	}
	else if (key == GLUT_KEY_LEFT) {
		scLeftNum++;
		scPositionX -= scLeftPace * cos(scLeftRotateNum * scRotatePace);
		scPositionZ += scLeftPace * sin(scLeftRotateNum * scRotatePace);
	}
	else if (key == GLUT_KEY_RIGHT) {
		scLeftNum--;
		scPositionX += scLeftPace * cos(scLeftRotateNum * scRotatePace);
		scPositionZ -= scLeftPace * sin(scLeftRotateNum * scRotatePace);
	}
}

void PassiveMouse(int x, int y)
{
	//TODO: Use Mouse to do interactive events and animation
	if (!mousePositionIni) {
		originMouseX = x;
		mousePositionIni = true;
	}
	else {
		scLeftRotateNum = -0.5*(x - originMouseX);
	}   

}

bool loadOBJ(
	const char * path,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE * file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 6 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

				   // else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y; 
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i<vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}

	return true;
}


GLuint loadBMP_custom(const char * imagepath) {

	printf("Reading image %s\n", imagepath);

	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	unsigned char * data;

	FILE * file = fopen(imagepath, "rb");
	if (!file) { printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); return 0; }

	if (fread(header, 1, 54, file) != 54) {
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (*(int*)&(header[0x1E]) != 0) { printf("Not a correct BMP file\n");    return 0; }
	if (*(int*)&(header[0x1C]) != 24) { printf("Not a correct BMP file\n");    return 0; }

	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);
	if (imageSize == 0)    imageSize = width*height * 3; 
	if (dataPos == 0)      dataPos = 54; 

	data = new unsigned char[imageSize];
	fread(data, 1, imageSize, file);
	fclose(file);

	
	GLuint textureID;
	glGenTextures(1, &textureID);//dont forget
	//TODO: Create one OpenGL texture and set the texture parameter 
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glGenerateMipmap(GL_TEXTURE_2D);
	
	return textureID;
}

GLuint loadSkyboxBMP() {
	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height;
	unsigned char* image;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < faces.size(); i++) {
		printf("Reading image %s\n", faces[i]);

		unsigned char header[54];
		unsigned int dataPos;
		unsigned int imageSize;
		unsigned int width, height;
		unsigned char * data;

		FILE * file = fopen(faces[i], "rb");
		if (!file) { printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", faces[i]); getchar(); return 0; }

		if (fread(header, 1, 54, file) != 54) {
			printf("Not a correct BMP file\n");
			return 0;
		}
		if (header[0] != 'B' || header[1] != 'M') {
			printf("Not a correct BMP file\n");
			return 0;
		}
		if (*(int*)&(header[0x1E]) != 0) { printf("Not a correct BMP file\n");    return 0; }
		if (*(int*)&(header[0x1C]) != 24) { printf("Not a correct BMP file\n");    return 0; }

		dataPos = *(int*)&(header[0x0A]);
		imageSize = *(int*)&(header[0x22]);
		width = *(int*)&(header[0x12]);
		height = *(int*)&(header[0x16]);
		if (imageSize == 0)    imageSize = width*height * 3;
		if (dataPos == 0)      dataPos = 54;

		data = new unsigned char[imageSize];
		fread(data, 1, imageSize, file);
		fclose(file);

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
		delete[] data;
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;


}

void sendDataToOpenGL()
{
	vector <vec3> vertices;
	vector <vec2> uvs;
	vector <vec3> normals;

	//The text plane

	const GLfloat plane[] = {
		//  position			color		
		-1.0f,+0.0f,-1.0f,	+0.0f,+1.0f,+0.0f,//0
		-1.0f,+0.0f,+1.0f,	+0.0f,+1.0f,+0.0f,//1
		+1.0f,+0.0f,+1.0f,	+0.0f,+1.0f,+0.0f,//2

		-1.0f,+0.0f,-1.0f,	+0.0f,+1.0f,+0.0f,//0
		+1.0f,+0.0f,+1.0f,	+0.0f,+1.0f,+0.0f,//2
		+1.0f,+0.0f,-1.0f,	+0.0f,+1.0f,+0.0f,//3
	};

	//test plane. For debug.
	glGenVertexArrays(1, &testPlaneVAO);
	glBindVertexArray(testPlaneVAO);

	glGenBuffers(1, &testPlaneVBO);
	glBindBuffer(GL_ARRAY_BUFFER, testPlaneVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(plane), plane, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (char*)(3 * sizeof(float)));

	//The plane
	loadOBJ("sources/spaceCraft.obj", vertices, uvs, normals);
	glGenVertexArrays(1, &spaceCraftVAO);
	glBindVertexArray(spaceCraftVAO);

	glGenBuffers(1, &vertixBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertixBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glGenBuffers(1, &uvBO);
	glBindBuffer(GL_ARRAY_BUFFER, uvBO);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2), &uvs[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glGenBuffers(1, &normalBO);
	glBindBuffer(GL_ARRAY_BUFFER, normalBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), &normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	drawSize_spaceCraft = vertices.size();
	vertices.clear(); uvs.clear(); normals.clear();//clear all three vector.

	//=================================LOAD TEXTURE========================================//
	spaceCraftTexture = loadBMP_custom("sources/texture/spacecraftTexture.bmp");

	
	//Earth---------------------------------------------------------------------------------------
	loadOBJ("sources/planet.obj", vertices, uvs, normals);
	glGenVertexArrays(1, &earthVAO);
	glBindVertexArray(earthVAO);

	glGenBuffers(1, &vertixBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertixBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glGenBuffers(1, &uvBO);
	glBindBuffer(GL_ARRAY_BUFFER, uvBO);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2), &uvs[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glGenBuffers(1, &normalBO);
	glBindBuffer(GL_ARRAY_BUFFER, normalBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), &normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	drawSize_earth = vertices.size();
	vertices.clear(); uvs.clear(); normals.clear();
	
	earthTexture = loadBMP_custom("sources/texture/earthTexture.bmp");
	earthNormalTex = loadBMP_custom("sources/texture/earth_normal.bmp");
	
	//wonderStar----------------------------------------------------------------------------------
	loadOBJ("sources/planet.obj", vertices, uvs, normals);
	glGenVertexArrays(1, &wonderStarVAO);
	glBindVertexArray(wonderStarVAO);

	glGenBuffers(1, &vertixBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertixBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glGenBuffers(1, &uvBO);
	glBindBuffer(GL_ARRAY_BUFFER, uvBO);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2), &uvs[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glGenBuffers(1, &normalBO);
	glBindBuffer(GL_ARRAY_BUFFER, normalBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), &normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	drawSize_wonderStar = vertices.size();
	vertices.clear(); uvs.clear(); normals.clear();

	wonderStarTexture = loadBMP_custom("sources/texture/WonderStarTexture.bmp");

	//rings, one object, move 4 times, draw one at each position to give 4 rings------------------
	loadOBJ("sources/Ring.obj", vertices, uvs, normals);
	glGenVertexArrays(1, &ringVAO);
	glBindVertexArray(ringVAO);

	glGenBuffers(1, &vertixBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertixBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glGenBuffers(1, &uvBO);
	glBindBuffer(GL_ARRAY_BUFFER, uvBO);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2), &uvs[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glGenBuffers(1, &normalBO);
	glBindBuffer(GL_ARRAY_BUFFER, normalBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), &normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	drawSize_ring = vertices.size();
	vertices.clear(); uvs.clear(); normals.clear();

	ringTexture = loadBMP_custom("sources/texture/ringTexture.bmp");
	changedTexture = loadBMP_custom("sources/texture/changedTex.bmp");

	//rock----------------------------------------------------------------------
	loadOBJ("sources/rock.obj", vertices, uvs, normals);
	glGenVertexArrays(1, &rockVAO);
	glBindVertexArray(rockVAO);

	glGenBuffers(1, &vertixBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertixBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glGenBuffers(1, &uvBO);
	glBindBuffer(GL_ARRAY_BUFFER, uvBO);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2), &uvs[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glGenBuffers(1, &normalBO);
	glBindBuffer(GL_ARRAY_BUFFER, normalBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), &normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	drawSize_rock = vertices.size();
	vertices.clear(); uvs.clear(); normals.clear();

	rockTexture = loadBMP_custom("sources/texture/rockTexture.bmp");


	GLfloat skyboxVertices[] = {
		-skyVertex,  skyVertex, -skyVertex, 
		-skyVertex, -skyVertex, -skyVertex, 
		 skyVertex, -skyVertex, -skyVertex,
		 skyVertex, -skyVertex, -skyVertex,
		 skyVertex,  skyVertex, -skyVertex,
		-skyVertex,  skyVertex, -skyVertex,

		-skyVertex, -skyVertex,  skyVertex,
		-skyVertex, -skyVertex, -skyVertex,
		-skyVertex,  skyVertex, -skyVertex,
		-skyVertex,  skyVertex, -skyVertex,
		-skyVertex,  skyVertex,  skyVertex, 
		-skyVertex, -skyVertex,  skyVertex,

		 skyVertex, -skyVertex, -skyVertex, 
		 skyVertex, -skyVertex,  skyVertex, 
		 skyVertex,  skyVertex,  skyVertex, 
		 skyVertex,  skyVertex,  skyVertex, 
		 skyVertex,  skyVertex, -skyVertex, 
		 skyVertex, -skyVertex, -skyVertex, 

		-skyVertex, -skyVertex,  skyVertex, 
		-skyVertex,  skyVertex,  skyVertex, 
		 skyVertex,  skyVertex,  skyVertex, 
		 skyVertex,  skyVertex,  skyVertex, 
		 skyVertex, -skyVertex,  skyVertex, 
		-skyVertex, -skyVertex,  skyVertex, 

		-skyVertex,  skyVertex, -skyVertex, 
		 skyVertex,  skyVertex, -skyVertex, 
		 skyVertex,  skyVertex,  skyVertex, 
		 skyVertex,  skyVertex,  skyVertex, 
		-skyVertex,  skyVertex,  skyVertex, 
		-skyVertex,  skyVertex, -skyVertex, 

		-skyVertex, -skyVertex, -skyVertex, 
		-skyVertex, -skyVertex,  skyVertex, 
		 skyVertex, -skyVertex, -skyVertex, 
		 skyVertex, -skyVertex, -skyVertex,
		-skyVertex, -skyVertex,  skyVertex, 
		 skyVertex, -skyVertex,  skyVertex, 
	};

	const GLfloat skyUV[] = {
		-1.0f, -1.0f, 
	};

	const GLfloat skyNormal[] = {
		+1.0f, +1.0f, +1.0f,
	};

	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);

	glGenBuffers(1, &uvBO);
	glBindBuffer(GL_ARRAY_BUFFER, uvBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyUV), skyUV, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

	/*glGenBuffers(1, &normalBO);
	glBindBuffer(GL_ARRAY_BUFFER, normalBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyNormal), skyNormal, GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);*/

	faces.push_back("sources/texture/universe_skybox/purplenebula_rt.bmp");
	faces.push_back("sources/texture/universe_skybox/purplenebula_lf.bmp");
	faces.push_back("sources/texture/universe_skybox/purplenebula_up.bmp");
	faces.push_back("sources/texture/universe_skybox/purplenebula_dn.bmp");
	faces.push_back("sources/texture/universe_skybox/purplenebula_bk.bmp");
	faces.push_back("sources/texture/universe_skybox/purplenebula_ft.bmp");

	skyboxTex = loadSkyboxBMP();
	vertices.clear(); uvs.clear(); normals.clear();



}

void setMatrix(string name, int Indicator) {
	mat4 modelScalingMatrix = mat4(1.0f);
	mat4 modelTransformMatrix = mat4(1.0f);
	mat4 modelRotationMatrix = mat4(1.0f);
	mat4 lookAt = mat4(1.0f);

	mat4 projectionMatrix = glm::perspective(20.0f, 1.0f, 7.0f, 10000.0f);

	GLint projectiveMatrixUniformLocation = glGetUniformLocation(programID, "projectionMatrix");
	glUniformMatrix4fv(projectiveMatrixUniformLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

	lookAt = glm::lookAt(
		vec3(scPositionX + 100 * sin(scLeftRotateNum * scRotatePace), 40.0f, scPositionZ + 100 * cos(scLeftRotateNum * scRotatePace)),
		vec3(scPositionX, 40.0f, scPositionZ),
		vec3(0.0f, 1.0f, 0.0f));


	if (name == "testPlane") {
		modelScalingMatrix = glm::scale(mat4(), vec3(5.0f, 5.0f, 5.0f));//2*2
		modelTransformMatrix = glm::translate(mat4(), vec3(0.0f, -2.0f, -15.0f));
	}
	if (name == "spaceCraft") {
		modelScalingMatrix = glm::scale(mat4(), vec3(0.05f, 0.07f, 0.105f));
		modelRotationMatrix = glm::rotate(mat4(),
			3.14f + scLeftRotateNum * scRotatePace,
			vec3(0, 1, 0));
		modelTransformMatrix = glm::translate(mat4(),
			vec3(scPositionX, 0.0f, scPositionZ));
	}
	if (name == "earth") {
		modelScalingMatrix = glm::scale(mat4(), vec3(20.0f, 30.0f, 20.0f));
		modelRotationMatrix = glm::rotate(mat4(), rotationParam, vec3(0, 1, 0));
		modelTransformMatrix = glm::translate(mat4(), earthPosition);
	}
	if (name == "wonderStar") {
		modelScalingMatrix = glm::scale(mat4(), vec3(20.0f, 30.0f, 20.0f));
		modelRotationMatrix = glm::rotate(mat4(), -rotationParam, vec3(0, 1, 0));
		modelTransformMatrix = glm::translate(mat4(), wonderStarPosition);
	}
	if (name == "ring") {
		modelScalingMatrix = glm::scale(mat4(), vec3(1.0f, 0.5f, 1.5f));
		modelRotationMatrix = glm::rotate(mat4(), 1.57f, vec3(1, 0, 0));
		modelRotationMatrix *= glm::rotate(mat4(), 5*rotationParam, vec3(0, 0, 1));
		modelTransformMatrix = glm::translate(mat4(), ringPosition[Indicator]);
	}
	if (name == "skybox") {
		modelScalingMatrix = glm::scale(mat4(), vec3(1.0f, 1.0f, 1.0f));
		modelRotationMatrix = glm::rotate(mat4(), 0.0f, vec3(0, 1, 0));
		modelTransformMatrix = glm::translate(mat4(), vec3(0.0f, 0.0f, 0.0f));
	}
	if (name == "rock") {
		modelScalingMatrix = modelMatrices[Indicator];
		modelRotationMatrix = glm::rotate(mat4(), rotationParam, vec3(0, 1, 0));
		modelTransformMatrix =  glm::translate(mat4(), rockTransformVector);
	}


	GLint transformMatrixUniformLocation = glGetUniformLocation(programID, "modelTranformMatrix");
	glUniformMatrix4fv(transformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);

	GLint scaleMatrixUniformLocation = glGetUniformLocation(programID, "modelScalingMatrix");
	glUniformMatrix4fv(scaleMatrixUniformLocation, 1, GL_FALSE, &modelScalingMatrix[0][0]);

	GLint rotateMatrixUniformLocation = glGetUniformLocation(programID, "modelRotationMatrix");
	glUniformMatrix4fv(rotateMatrixUniformLocation, 1, GL_FALSE, &modelRotationMatrix[0][0]);

	GLint LookAtMatrixUniformLocation = glGetUniformLocation(programID, "lookAtMatrix");
	glUniformMatrix4fv(LookAtMatrixUniformLocation, 1, GL_FALSE, &lookAt[0][0]);


	//lighting
	//ambient light
	GLint ambientLightUniformLocation = glGetUniformLocation(programID, "ambientLight");
	vec3 ambientLight(amb, amb, amb);
	glUniform3fv(ambientLightUniformLocation, 1, &ambientLight[0]);

	//diffuse light
	GLint lightPositionUniformLocation = glGetUniformLocation(programID, "lightPositionWorld");
	vec3 lightPosition(0.0f + lightx, 8.0f, 0.0f + lightz);
	glUniform3fv(lightPositionUniformLocation, 1, &lightPosition[0]);

	GLint diffuseLightBrightUniformLocation = glGetUniformLocation(programID, "diffbri");
	float diffbri = dif;
	glUniform1f(diffuseLightBrightUniformLocation, diffbri);

	//specular light
	GLint eyePositionUniformLocation = glGetUniformLocation(programID, "eyePositionWorld");
	vec3 eyePosition(0.0f, 0.0f, 0.0f);
	glUniform3fv(eyePositionUniformLocation, 1, &lightPosition[0]);

	GLint specLightBrightUniformLocation = glGetUniformLocation(programID, "specbri");
	float specbri = spec;
	glUniform1f(specLightBrightUniformLocation, specbri);



}

void paintGL(void)
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	//========================================================
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	//TextureID = glGetUniformLocation(programID, "myTextureSampler");
	rotationParam += rotationSpeed;

	//skybox front
	setInt("skyboxDisting", (int)1);
	setInt("skydis", (int)1);
	glDepthMask(GL_FALSE);
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	setMatrix("skybox", 0);
	glUniform1i(glGetUniformLocation(programID, "skybox"), 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTex);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	//glBindVertexArray(0);
	glDepthMask(GL_TRUE);
	setInt("skyboxDisting", 0);
	setInt("skydis", 0);

	//set the disappear array
	for (int i = 0; i < rocksNum; i++) {
		rockPosition[i] = vec3(
			rockXYZ[i].x * cos(-rotationParam) - rockXYZ[i].z * sin(-rotationParam),
			rockXYZ[i].y,
			rockXYZ[i].x*sin(-rotationParam) + rockXYZ[i].z*cos(-rotationParam)
		) + rockTransformVector;
	}

	distanceLessThan(4, 100.0f);
	distanceLessThan(5, 100.0f);
	for (int i = 0; i < 200; i++) {
		distanceLessThan(i + 6, 50.0f);
	}

	//four rings
	for (int i = 0; i < 4; i++) {
		glBindVertexArray(ringVAO);
		setMatrix("ring", i);
		if (distanceLessThan(i, 50.0f)) {
			spaceCraftChangeTexture += true;
			glBindTexture(GL_TEXTURE_2D, changedTexture);
		}
		else {
			spaceCraftChangeTexture += false;
			glBindTexture(GL_TEXTURE_2D, ringTexture);
		}
		//
		glDrawArrays(GL_TRIANGLES, 0, drawSize_ring);
	}



	//space craft
	glBindVertexArray(spaceCraftVAO);
	setMatrix("spaceCraft", 0);
	//glActiveTexture(GL_TEXTURE0);
	if (spaceCraftChangeTexture) {
		glBindTexture(GL_TEXTURE_2D, changedTexture);
	}
	else
		glBindTexture(GL_TEXTURE_2D, spaceCraftTexture);
	//glUniform1i(TextureID, 0);
	glDrawArrays(GL_TRIANGLES, 0, drawSize_spaceCraft);
	spaceCraftChangeTexture = false;

	//earth
	glBindVertexArray(earthVAO);
	setInt("normalDis", 1);
	GLuint TextureID_1 = glGetUniformLocation(programID, "myTextureSampler_2");
	setMatrix("earth", 0);
	//
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthNormalTex);
	glUniform1i(TextureID_1, 1);
	//
	if(objectDisappear[4]==0)
		glDrawArrays(GL_TRIANGLES, 0, drawSize_earth);
	setInt("normalDis", 0);
	glActiveTexture(GL_TEXTURE0);

	//wonderStar
	glBindVertexArray(wonderStarVAO);
	setMatrix("wonderStar", 0);
	//
	glBindTexture(GL_TEXTURE_2D, wonderStarTexture);
	//
	if (objectDisappear[5] == 0)
		glDrawArrays(GL_TRIANGLES, 0, drawSize_wonderStar);
	

	//Rocks

	for (int i = 0; i < rocksNum; i++) {
		glBindVertexArray(rockVAO);
		setMatrix("rock", i);
		glBindTexture(GL_TEXTURE_2D, rockTexture);
		if(objectDisappear[i+6]==0)
			glDrawArrays(GL_TRIANGLES, 0, drawSize_rock);
	}



	glFlush();
	glutPostRedisplay();
}

void initializedGL(void) //run only once
{
	glewInit();
	installShaders();
	sendDataToOpenGL();
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);

	glutInitWindowSize(1080, 720);
	glutCreateWindow("Final Pro");
	createModelMatrices();
	
	//TODO:
	/*Register different CALLBACK function for GLUT to response
	with different events, e.g. window sizing, mouse click or
	keyboard stroke */
	initializedGL();
	glutDisplayFunc(paintGL);
	
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(move);
	glutPassiveMotionFunc(PassiveMouse);
	
	glutMainLoop();

	return 0;
}