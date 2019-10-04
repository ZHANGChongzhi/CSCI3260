#version 430  // GLSL version your computer supports

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 vertexColor;

uniform mat4 projectionMatrix;

uniform mat4 modelTranformMatrix;
uniform mat4 modelScalingMatrix;
uniform mat4 modelRotationMatrix;

out vec3 theColor;


void main()
{
	vec4 v=vec4(position,1.0);
	vec4 new_position = projectionMatrix * modelTranformMatrix * modelScalingMatrix * modelRotationMatrix * v;
	gl_Position = new_position;
	theColor = vertexColor;
}