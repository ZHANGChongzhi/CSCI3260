#version 430  // GLSL version your computer supports
in layout(location = 0) vec3 position;
in layout(location = 1) vec2 vertexUV;
in layout(location = 2) vec3 normal;

uniform mat4 projectionMatrix;
uniform mat4 modelTranformMatrix;
uniform mat4 modelScalingMatrix;
uniform mat4 modelRotationMatrix;
uniform mat4 lookAt;

uniform vec3 ambientLight;

out vec2 frag_UV;
out vec3 frag_color;
out vec3 frag_ambientLight;
out vec3 frag_normalWorld;
out vec3 frag_vertexPositionWorld;

void main()
{
	frag_UV = vertexUV;
	frag_ambientLight = ambientLight;

	vec4 v=vec4(position,1.0);
	vec4 new_position =  modelTranformMatrix * modelScalingMatrix * modelRotationMatrix * v;
	gl_Position =  lookAt * projectionMatrix * new_position;

	vec4 normal_temp = modelTranformMatrix * modelScalingMatrix * modelRotationMatrix * vec4(normal,0.0);
	frag_normalWorld = normal_temp.xyz;

	frag_vertexPositionWorld = new_position.xyz;
}