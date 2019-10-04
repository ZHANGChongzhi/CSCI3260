#version 430  // GLSL version your computer supports

in layout(location = 0) vec3 position;
in layout(location = 1) vec2 vertexUV;
in layout(location = 2) vec3 normal;

uniform mat4 projectionMatrix;

uniform mat4 modelTranformMatrix;
uniform mat4 modelScalingMatrix;
uniform mat4 modelRotationMatrix;
uniform mat4 lookAtMatrix;

uniform int skyboxDisting;

out vec3 normalWorld;
out vec2 UV;
out vec3 vertexPositionWorld;

void main()
{

	if(skyboxDisting == 1){
		gl_Position =   projectionMatrix *lookAtMatrix * vec4(position, 1.0);  
		normalWorld = position;
		//printf("skybox vertex shader done!\n");
	}
	else{
		vec4 v=vec4(position,1.0);
		vec4 new_position = projectionMatrix *lookAtMatrix * modelTranformMatrix * modelRotationMatrix * modelScalingMatrix * v;
		gl_Position = new_position;



		vec4 normal_temp = modelTranformMatrix * vec4(normal, 0);
		normalWorld = normal_temp.xyz;

		vec4 transformpos = modelTranformMatrix * modelScalingMatrix * modelRotationMatrix * v;
		vertexPositionWorld = transformpos.xyz;

		UV = vertexUV;
	}



}