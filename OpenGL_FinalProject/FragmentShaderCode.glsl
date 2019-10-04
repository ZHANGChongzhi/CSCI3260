#version 430 //GLSL version your computer supports

in vec2 UV;
in vec3 normalWorld;
in vec3 vertexPositionWorld;

uniform sampler2D myTextureSampler;
uniform sampler2D myTextureSampler_2;

uniform vec3 ambientLight;
uniform vec3 lightPositionWorld;
uniform vec3 eyePositionWorld;
uniform float diffbri;
uniform float specbri;

uniform samplerCube skybox;
uniform int skydis;

uniform int normalDis;

out vec4 dacolor;

void main()
{
	vec3 normal = normalize(normalWorld);
	if(normalDis == 1){
		normal = texture(myTextureSampler_2, UV).rgb;
		normal = normalize(normal * 2.0 - 1.0);
	}

	if(skydis == 1){
		dacolor = texture(skybox, normalWorld);
		//printf("skybox fragment shader done\n");
	}
	else{
		//diffuse
		vec3 lightVectorWorld = normalize(lightPositionWorld - vertexPositionWorld);
		float brightness = dot(lightVectorWorld, normal);
		brightness *= diffbri;
		vec4 diffuseLight  = vec4(brightness, brightness, brightness, 1.0);
	
		//specular
		vec3 reflectedLightVectorWorld = reflect(-lightVectorWorld, normal);
		vec3 eyeVectorWorld = normalize(eyePositionWorld - vertexPositionWorld);
		float s = clamp(dot(reflectedLightVectorWorld, eyeVectorWorld), 0, 1);
		s = pow(s, 10);
		s *= specbri;
		s = clamp(s, 0, 1);
		vec4 specularLight = vec4(s, s, s, 1);

		vec4 materialSpecularColor = vec4(1.0, 1.0, 1.0, 1.0);

		//ambient
		vec4 ambient = vec4(ambientLight, 1.0);

		vec4 color = vec4(texture(myTextureSampler, UV).rgb, 1.0f);
		dacolor = color * ambientLight + color * diffuseLight * brightness + materialSpecularColor * specularLight * s;
		//dacolor = color * ambientLight;
		//dacolor = color * diffuseLight * brightness;
		//dacolor = materialSpecularColor * specularLight * s;
	}
	
}


