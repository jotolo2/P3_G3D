#version 330 core

out vec4 outColor;

in vec3 color;
in vec3 pos;
in vec3 norm;
in vec2 texCoord;

uniform sampler2D colorTex;
uniform sampler2D emiTex;

//Propiedades de la luz
uniform vec4 lightPos;
uniform vec3 Ia;
uniform vec3 Id;
uniform vec3 Is;

//Propiedades del objeto
vec3 Ka;
vec3 Kd;
vec3 Ks;
vec3 N;
float alpha = 500.0;

vec3 shade();

void main()
{
	Ka = texture(colorTex, texCoord).rgb;
	Kd = texture(colorTex, texCoord).rgb;
	Ks = vec3 (1.0);

	N = normalize (norm);
	outColor = vec4(shade(), 1.0);   
}

vec3 shade()
{
	vec3 c = Ia * Ka;
	vec3 lpos = lightPos.xyz;

	vec3 L = normalize (lpos - pos);
	vec3 diffuse = Id * Kd * dot (L,N);
	c += clamp(diffuse, 0.0, 1.0);
	
	vec3 V = normalize (-pos);
	vec3 R = normalize (reflect (-L,N));
	float factor = max (dot (R,V), 0.01);
	vec3 specular = Is*Ks*pow(factor,alpha);
	c += clamp(specular, 0.0, 1.0);
	
	return c;
}
