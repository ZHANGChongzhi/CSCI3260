/*********************************************************
FILE : main.cpp (csci3260 2018-2019 Assignment 2)
*********************************************************/
/*********************************************************
Student Information
Student ID:1155077072
Student Name:ZHANG Chongzhi
*********************************************************/

#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"
#include "Dependencies\glm\glm.hpp"
#include "Dependencies\glm\gtc\matrix_transform.hpp"
#include "Dependencies\glm\gtc\type_ptr.hpp"
#include <iostream>
#include <fstream>
#include <vector>
  
#include <time.h>
using namespace std;
using glm::vec3;
using glm::vec2;
using glm::vec4;
using glm::mat4;

GLint programID;
GLuint TextureID;

GLuint testPlaneVAO, planeVAO, jeepVAO, blockVAO, pointVAO;
GLuint testPlaneVBO, pointVBO;//VBO of testPlane.
GLuint vertixBO, uvBO, normalBO;//BOs(VBO UVBO NBO) of other objects.

GLuint planeTexture[3], jeepTexture, blockTexture;//keep textureID used in loadBMP().
GLsizei drawSize_plane, drawSize_jeep, drawSize_block;//vertices number of each object.

//animation ralated variables

int THEME = 1;//1,2,3
float ambientBrightness = 5,diffuseBrightness = 1,specularBrightness = 1;//ajust by e,r
int lightForwordNum = 0, lightLeftNum = 0, lightUpNum = 0;//i,j,k,l,o,p to control
float lightMovePace = 0.6f;

bool enableMouseControl = true, mousePositionIni = false;// ajust by SPACE
int oragenMouseX, oragenMouseY;//keep the former mouse situation
float lookUp = 0, lookLeft = 0;//paramiter used in mat4 LookAt



//vec3 jeepDirection = vec3(1.0f, 0.0f, 0.0f);//keep the direction if jeep
//vec3 jeepPosition = vec3(0.0f);//assume the stating position is 0,0,0
//vec4 jeepDirection4, jeepPosition4;//4D vector, same as above.
float jeepRotatePace = 0.1f, jeepForwardPace = 0.3f;//can be ajusted.
int jeepLeftRotateNum = 0, jeepForwardNum = 0;//use arrow key to control
float jeepPositionX = 0, jeepPositionZ = 0;//keep the position of jeep.

bool blockStartRotation = true; //use key s to shift.
float blockRotatePace = 0.01f;
int blockRotateNum = 0;
float blockPositionX = 0, blockPositionZ = 0;

vector<GLfloat> pointPositionInf;

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


// Start form below.

void keyboard(unsigned char key, int x, int y)
{
	//TODO: Use keyboard to do interactive events and animation
	if (key == '1') {
		THEME = 1;
	}
	if (key == '2') {
		THEME = 2;
	}
	if (key == '3') {
		THEME = 3;
	}
	if (key == 's') {
		blockStartRotation = !blockStartRotation;
	}
	if (key == 'e') {
		if(ambientBrightness < 10)
			ambientBrightness++;
	}
	if (key == 'r') {
		if (ambientBrightness > 0)
			ambientBrightness--;
	}
	//move the light
	if (key == ' ') {
		enableMouseControl = !enableMouseControl;
		mousePositionIni = false;
	}
	if (key == 'i') {
		lightForwordNum++;
	}
	if (key == 'k') {
		lightForwordNum--;
	}
	if (key == 'j') {
		lightLeftNum++;
	}
	if (key == 'l') {
		lightLeftNum--;
	}
	if (key == 'o') {
		lightUpNum++;
	}
	if (key == 'p') {
		lightUpNum--;
	}
	if (key == 'q') {
		diffuseBrightness += 0.1;
	}
	if (key == 'w') {
		diffuseBrightness -= 0.1;
	}
	if (key == 'z') {
		specularBrightness += 0.1;
	}
	if (key == 'x') {
		specularBrightness -= 0.1;
	}

}

void move(int key, int x, int y) 
{
	//TODO: Use arrow keys to do interactive events and animation
	//calc the direction
	//jeepDirection4 = vec4(jeepDirection, 1.0f)* glm::rotate(mat4(), jeepRotatePace*jeepLeftRotateNum, vec3(0, 1, 0));
	//jeepDirection = vec3(jeepDirection4);

	if (key == GLUT_KEY_UP) {
		jeepForwardNum++;
		//jeepPosition += jeepForwardPace * jeepDirection;
		jeepPositionX += jeepForwardPace * cos(jeepLeftRotateNum * jeepRotatePace);
		jeepPositionZ -= jeepForwardPace * sin(jeepLeftRotateNum * jeepRotatePace);
		//track the jeep route
		pointPositionInf.push_back(jeepPositionX);
		pointPositionInf.push_back(1.0f);
		pointPositionInf.push_back(jeepPositionZ);
	}
	else if (key == GLUT_KEY_DOWN) {
		jeepForwardNum--;
		//jeepPosition -= jeepForwardPace * jeepDirection;
		jeepPositionX -= jeepForwardPace * cos(jeepLeftRotateNum * jeepRotatePace);
		jeepPositionZ += jeepForwardPace * sin(jeepLeftRotateNum * jeepRotatePace);
		//track the jeep route
		pointPositionInf.push_back(jeepPositionX);
		pointPositionInf.push_back(1.0f);
		pointPositionInf.push_back(jeepPositionZ);
	}
	else if (key == GLUT_KEY_LEFT) {
		jeepLeftRotateNum++;
	}
	else if (key == GLUT_KEY_RIGHT) {
		jeepLeftRotateNum--;
	}

}

void PassiveMouse(int x, int y)
{
	//TODO: Use Mouse to do interactive events and animation
	if (enableMouseControl) {
		if (!mousePositionIni) {
			oragenMouseX = x;
			oragenMouseY = y;
			mousePositionIni = true;
		}
		else{
			lookUp = -(y - oragenMouseY);
			lookLeft = x - oragenMouseX;
		}
	}

}

bool loadOBJ(const char * path,vector<vec3> & out_vertices,vector<vec2> & out_uvs,vector<vec3> & out_normals) {
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
			int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
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

void sendDataToOpenGL()
{
	//TODO:
	//Load objects and bind to VAO & VBO
	//Load texture
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
	loadOBJ("sources/plane.obj", vertices, uvs, normals);
	glGenVertexArrays(1, &planeVAO);
		glBindVertexArray(planeVAO);

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

	drawSize_plane = vertices.size();
	vertices.clear(); uvs.clear(); normals.clear();

	//The jeep
	loadOBJ("sources/jeep.obj", vertices, uvs, normals);
	glGenVertexArrays(1, &jeepVAO);
	glBindVertexArray(jeepVAO);

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

	drawSize_jeep = vertices.size();
	vertices.clear(); uvs.clear(); normals.clear();

	//The block
	loadOBJ("sources/block.obj", vertices, uvs, normals);
	glGenVertexArrays(1, &blockVAO);
	glBindVertexArray(blockVAO);

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

	drawSize_block = vertices.size();
	vertices.clear(); uvs.clear(); normals.clear();



	//=================================LOAD TEXTURE========================================//
	blockTexture = loadBMP_custom("sources/block_texture.bmp");
	jeepTexture = loadBMP_custom("sources/jeep_texture.bmp");
	planeTexture[0] = loadBMP_custom("sources/theme1.bmp");
	planeTexture[1] = loadBMP_custom("sources/theme2.bmp");
	planeTexture[2] = loadBMP_custom("sources/theme3.bmp");

}

void setMatrix(string name) {
	mat4 modelScalingMatrix = mat4(1.0f);
	mat4 modelTransformMatrix = mat4(1.0f);
	mat4 modelRotationMatrix = mat4(1.0f);
	mat4 lookAt = glm::translate(mat4(), vec3(-0.002*lookLeft, -0.002*lookUp, 0.0f));
	//projection all the same.
	mat4 projectionMatrix = glm::perspective(20.0f, 1.5f, 1.0f, 100.0f);

	GLint projectiveMatrixUniformLocation = glGetUniformLocation(programID, "projectionMatrix");
	glUniformMatrix4fv(projectiveMatrixUniformLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
	GLint lookAtMatrixUniformLocation = glGetUniformLocation(programID, "lookAt");
	glUniformMatrix4fv(lookAtMatrixUniformLocation, 1, GL_FALSE, &lookAt[0][0]);

	if (name == "testPlane") {
		modelScalingMatrix = glm::scale(mat4(), vec3(5.0f, 5.0f, 5.0f));//2*2
		modelTransformMatrix = glm::translate(mat4(), vec3(0.0f, -5.0f, -9.0f));
	}
	if (name == "plane") {
		modelScalingMatrix = glm::scale(mat4(), vec3(3.0f, 3.0f, 3.0f));//2*2
		modelTransformMatrix = glm::translate(mat4(), vec3(0.0f, -5.5f, -20.0f));
	}
	if (name == "jeep") {
		modelRotationMatrix = glm::rotate(mat4(), jeepRotatePace*jeepLeftRotateNum, vec3(0, 1, 0));
		modelTransformMatrix = glm::translate(mat4(), vec3(0.0f, -5.0f, -15.0f)
			+ vec3(jeepPositionX, 0.0f, jeepPositionZ));
	}
	if (name == "block") {
		if (clock()%5==0 && blockStartRotation == true) {
			blockRotateNum++;
		}
		blockPositionX = -7 * sin(blockRotatePace*blockRotateNum);
		blockPositionZ = -7 * cos(blockRotatePace*blockRotateNum);

		modelRotationMatrix = glm::rotate(mat4(), blockRotatePace*blockRotateNum, vec3(0, 1, 0));
		modelTransformMatrix = glm::translate(mat4(), vec3(0.0f, -2.0f, -15.0f)
			+ vec3(jeepPositionX, 0.0f, jeepPositionZ)
			+ vec3(blockPositionX, 0.0f, blockPositionZ));
	}

	GLint transformMatrixUniformLocation = glGetUniformLocation(programID, "modelTranformMatrix");
	glUniformMatrix4fv(transformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);

	GLint scaleMatrixUniformLocation = glGetUniformLocation(programID, "modelScalingMatrix");
	glUniformMatrix4fv(scaleMatrixUniformLocation, 1, GL_FALSE, &modelScalingMatrix[0][0]);

	GLint rotateMatrixUniformLocation = glGetUniformLocation(programID, "modelRotationMatrix");
	glUniformMatrix4fv(rotateMatrixUniformLocation, 1, GL_FALSE, &modelRotationMatrix[0][0]);
}

void paintGL(void)
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	//TODO:
	//Set lighting information, such as position and color of lighting source
	//Set transformation matrix
	//Bind different textures
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	glEnable(GL_DEPTH_TEST);

	//=================================Set Light Inf========================================//
	//ambient light
	GLint ambientLightUniformLocation = glGetUniformLocation(programID, "ambientLight");
	vec3 ambientLight(0.1, 0.1f, 0.1f);
	ambientLight = ambientLight * ambientBrightness;
	glUniform3fv(ambientLightUniformLocation, 1, &ambientLight[0]);
	//diffuse light
	GLint lightPositionUniformLocation = glGetUniformLocation(programID, "lightPositionWorld");
	vec3 lightPosition(0.0f - lightLeftNum * lightMovePace,
						5.0f+ lightUpNum * lightMovePace,
						-20.0f - lightForwordNum * lightMovePace);
	glUniform3fv(lightPositionUniformLocation, 1, &lightPosition[0]);
	//specular light
	GLint eyePositionUniformLocation = glGetUniformLocation(programID, "eyePositionWorld");
	vec3 eyePosition(0.002*lookLeft, 0.002*lookUp, 0.0f);
	glUniform3fv(eyePositionUniformLocation, 1, &eyePosition[0]);
	
	//set the brightness of the light.
	GLuint diffuseUniformLocation = glGetUniformLocation(programID, "diffuseBrightness");
	glUniform1f(diffuseUniformLocation, diffuseBrightness);
	GLuint specularUniformLocation = glGetUniformLocation(programID, "specularBrightness");
	glUniform1f(specularUniformLocation, specularBrightness);

	//============================================================================
	glBindVertexArray(testPlaneVAO);
	setMatrix("testPlane");
	//glDrawArrays(GL_TRIANGLES, 0, 12);


	glBindVertexArray(planeVAO);
	setMatrix("plane");
	if (THEME == 1)glBindTexture(GL_TEXTURE_2D, planeTexture[0]);
	else if (THEME == 2)glBindTexture(GL_TEXTURE_2D, planeTexture[1]);
	else glBindTexture(GL_TEXTURE_2D, planeTexture[2]);
	glDrawArrays(GL_TRIANGLES, 0, drawSize_plane);

	glBindVertexArray(jeepVAO);
	setMatrix("jeep");
	glBindTexture(GL_TEXTURE_2D, jeepTexture);
	glDrawArrays(GL_TRIANGLES, 0, drawSize_jeep);
	
	glBindVertexArray(blockVAO);
	setMatrix("block");
	glBindTexture(GL_TEXTURE_2D, blockTexture);
	glDrawArrays(GL_TRIANGLES, 0, drawSize_block);


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

	glutInitWindowSize(1024, 1024);
	glutCreateWindow("Assignment 2");
	
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