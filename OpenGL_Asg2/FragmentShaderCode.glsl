#version 430 //GLSL version your computer supports

in vec3 frag_color;
in vec2 frag_UV;
in vec3 frag_ambientLight;
in vec3 frag_normalWorld;
in vec3 frag_vertexPositionWorld;

uniform sampler2D myTextureSampler;
uniform vec3 lightPositionWorld;
uniform vec3 eyePositionWorld;
uniform float diffuseBrightness;
uniform float specularBrightness;

out vec4 daColor;

void main()
{
	vec3 color = texture(myTextureSampler,frag_UV).rgb;
	

	vec3 lightVectorWorld = normalize(lightPositionWorld - frag_vertexPositionWorld);
	float brightness = dot(lightVectorWorld,normalize(frag_normalWorld));
	vec4 diffuseLight = vec4(brightness,brightness,brightness,1.0);

	vec3 reflectedLightVectorWorld = reflect(-lightVectorWorld,frag_normalWorld);
	vec3 eyeVectorWorld = normalize(eyePositionWorld - frag_vertexPositionWorld);
	float s = clamp (dot (reflectedLightVectorWorld,eyeVectorWorld) , 0, 1);
	s = pow(s,50);
	vec4 specularLight = vec4(s,s,s,1);

	daColor = vec4(color * (frag_ambientLight
							+ diffuseBrightness * clamp(diffuseLight, 0, 1) 
							+ specularBrightness * specularLight) 
							, 1.0);
}
