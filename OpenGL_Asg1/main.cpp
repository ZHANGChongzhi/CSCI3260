///////////////////////////////////////////
////Type your name and student ID here/////
////Name:ZHANG Chongzhi
////Student ID:1155077072

#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"
#include "Dependencies\glm\glm.hpp"
#include "Dependencies\glm\gtc\matrix_transform.hpp"
#include <iostream>
#include <fstream>
using namespace std;
using glm::vec3;
using glm::mat4;

GLuint pyramidVAO, planeVAO , pointVAO , lineVAO;
GLuint pyramidVBO, pyramidIndexVBO, planeVBO ,pointVBO,lineVBO;
int AD_press_number = 0, WS_press_number = 0;
int Q_press_number = 0, E_press_number = 0;

GLint programID;

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
	//adapter[0] = vertexShaderCode;
	string temp = readShaderCode("VertexShaderCode.glsl");
	adapter[0] = temp.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	//adapter[0] = fragmentShaderCode;
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
	//TODO:
	if (key == 'a') {
		AD_press_number += 1;
	}
	if (key == 'd') {
		AD_press_number -= 1;
	}
	if (key == 'w') {
		WS_press_number += 1;
	}
	if (key == 's') {
		WS_press_number -= 1;
	}
	if (key == 'q') {
		Q_press_number += 1;
	}
	if (key == 'e') {
		E_press_number += 1;
	}
}

void sendDataToOpenGL()
{
	//TODO:
	//create point, line, 2D object and 3D object here and bind to VAOs & VBOs
	const GLfloat plane[] = {
		//  position			color
		-1.0f,+0.0f,-1.0f,	+0.0f,+1.0f,+0.0f,//0
		-1.0f,+0.0f,+1.0f,	+0.0f,+1.0f,+0.0f,//1
		+1.0f,+0.0f,+1.0f,	+0.0f,+1.0f,+0.0f,//2

		-1.0f,+0.0f,-1.0f,	+0.0f,+1.0f,+0.0f,//0
		+1.0f,+0.0f,+1.0f,	+0.0f,+1.0f,+0.0f,//2
		+1.0f,+0.0f,-1.0f,	+0.0f,+1.0f,+0.0f,//3
	};
	const GLfloat pyramid[] =
	{
		//  position			color
		-0.5f,+0.0f,-0.5f,		+1.0f,+0.0f,+0.0f,//0
		+0.5f,+0.0f,-0.5f,		+0.0f,+1.0f,+0.0f,//1
		+0.0f,+0.0f,+0.5f,		+1.0f,+0.0f,+1.0f,//2
		+0.0f,+0.5f,+0.0f,		+0.0f,+0.0f,+0.0f,//3,top
	};
	GLushort pyramidIndices[] =
	{
		0,1,2,
		0,2,3,
		1,2,3,
		0,1,3,
	};

	const GLfloat point[] =
	{
		+0.0f,+0.0f,+0.0f,		+0.0f,+0.0f,+0.0f,
	};

	const GLfloat line[] =
	{
		+0.0f,+0.0f,+0.0f,		+0.0f,+0.0f,+0.0f,
		+0.0f,+1.0f,+0.0f,		+0.0f,+0.0f,+0.0f,
	};
	//pyramid
	glGenVertexArrays(1, &pyramidVAO);
		glBindVertexArray(pyramidVAO);

	glGenBuffers(1, &pyramidVBO);
		glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(pyramid), pyramid, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (char*)(3*sizeof(float)));
		//bind the indices
	glGenBuffers(1, &pyramidIndexVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pyramidIndexVBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pyramidIndices)*sizeof(GLushort), pyramidIndices, GL_STATIC_DRAW);

	//plane
	glGenVertexArrays(1, &planeVAO);
		glBindVertexArray(planeVAO);

	glGenBuffers(1, &planeVBO);
		glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(plane), plane, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (char*)(3 * sizeof(float)));

	//point
	glGenVertexArrays(1, &pointVAO);
		glBindVertexArray(pointVAO);

	glGenBuffers(1, &pointVBO);
		glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(point), point, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (char*)(3 * sizeof(float)));
	//line
	glGenVertexArrays(1, &lineVAO);
		glBindVertexArray(lineVAO);

	glGenBuffers(1, &lineVBO);
		glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (char*)(3 * sizeof(float)));

}

void setMatrix(string name) {
	mat4 modelScalingMatrix = mat4(1.0f);
	mat4 modelTransformMatrix = mat4(1.0f);
	mat4 modelRotationMatrix = mat4(1.0f);

	mat4 projectionMatrix = glm::perspective(20.0f+0.1f*WS_press_number, 1.0f, 1.0f, 100.0f);

	GLint projectiveMatrixUniformLocation = glGetUniformLocation(programID, "projectionMatrix");
	glUniformMatrix4fv(projectiveMatrixUniformLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

	if (name == "plane") {
		modelScalingMatrix = glm::scale(mat4(), vec3(5.0f, 5.0f, 5.0f));//2*2
		//modelRotationMatrix = glm::rotate(mat4(), 0.3f, vec3(1, 0, 0));
		modelTransformMatrix = glm::translate(mat4(), vec3(0.0f, -2.0f, -15.0f));
		//modelTransformMatrix = translationMatrix * rotationMatrix * scaleMatrix;
	}
	else if (name == "pyramid") {
		modelScalingMatrix = glm::scale(mat4(), vec3(3.0f+0.05f*E_press_number, 3.0f + 0.05f*E_press_number, 3.0f + 0.05f*E_press_number));
		modelRotationMatrix = glm::rotate(mat4(), 0.1f*Q_press_number, vec3(0,1,0));
		modelTransformMatrix = glm::translate(mat4(), vec3(1.0f - AD_press_number*0.1f, -2.0f, -15.0f));
	}
	else if (name == "line") {
		modelScalingMatrix = glm::scale(mat4(), vec3(0.0f, 3.0f, 0.0f));
		modelTransformMatrix = glm::translate(mat4(), vec3(2.0f, 0.0f, -15.0f));
	}
	else if (name == "point") {
		modelTransformMatrix = glm::translate(mat4(), vec3(3.0f, 0.0f, -15.0f));
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
	//TODO:
	//render your objects and control the transformation here
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	glEnable(GL_DEPTH_TEST);


	glBindVertexArray(planeVAO);
	setMatrix("plane");	
	glDrawArrays(GL_TRIANGLES, 0, 12);

	
	glBindVertexArray(pyramidVAO);
	setMatrix("pyramid");
	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(lineVAO);
	setMatrix("line");
	glLineWidth(1.5f);
	glDrawArrays(GL_LINES, 0, 2);

	glBindVertexArray(pointVAO);
	setMatrix("point");
	glEnable(GL_POINT_SMOOTH);
	glPointSize(10.0f);
	glDrawArrays(GL_POINTS, 0, 1);


	glutSwapBuffers();

	glFlush();
	glutPostRedisplay();
}

void initializedGL(void) //run only once
{
	sendDataToOpenGL();
	installShaders();
}

int main(int argc, char *argv[])
{
	/*Initialization*/
	glutInitDisplayMode(GLUT_RGBA);

	glutInit(&argc, argv);

	glutInitWindowSize(512, 512);
	glutCreateWindow("Assignment 1");
	glewInit();

	/*Register different CALLBACK function for GLUT to response
	with different events, e.g. window sizing, mouse click or
	keyboard stroke */
	initializedGL();
	glutDisplayFunc(paintGL);
	glutKeyboardFunc(keyboard);

	/*Enter the GLUT event processing loop which never returns.
	it will call different registered CALLBACK according
	to different events. */
	glutMainLoop();

	return 0;
}