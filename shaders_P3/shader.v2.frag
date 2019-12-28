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

//Propeidades de la fuente de luz focal
struct SpotLight {
	vec3 intensity;
	vec3 position;
	vec3 direction;

	float constant;
    float linear;
    float quadratic;

	float cutOff;
	float m;
};
uniform SpotLight spotLight;

//Propiedades del objeto
vec3 Ka;
vec3 Kd;
vec3 Ks;
vec3 Ke;
vec3 N;
float n = 100;
float alpha = 500.0;

vec3 shade();

void main()
{
	Ka = texture(colorTex, texCoord).rgb;
	Kd = texture(colorTex, texCoord).rgb;
	Ke = texture(emiTex, texCoord).rgb;
	Ks = vec3 (1.0);

	N = normalize (norm);

	outColor = vec4(shade(), 1.0);   
}

vec3 shade()
{
	//Ambiental
	vec3 c = Ia * Ka;

	//Luz puntual
	vec3 lpos = lightPos.xyz;
	vec3 L =  lpos - pos;
	float distance = length(L);
	float atenuation = min(1 / (spotLight.constant + spotLight.linear * distance + spotLight.quadratic * distance * distance), 1);
	L = normalize(L);

	//Difusa
	vec3 diffuse = atenuation * Id * Kd * dot (L,N);
	c += clamp(diffuse, 0.0, 1.0);
	
	//Especular
	vec3 V = normalize (-pos);
	vec3 R = normalize (reflect (-L,N));
	float factor = max (dot (R,V), 0.01);
	vec3 specular = atenuation*Is*Ks*pow(factor,alpha);
	c += clamp(specular, 0.0, 1.0);

	//Luz focal
	L = spotLight.position - pos;
	distance = length(L);
	atenuation = min(1 / (spotLight.constant + spotLight.linear * distance + spotLight.quadratic * distance * distance), 1);
	L = normalize(L);
	float theta = dot(normalize(spotLight.direction), -L);

	if(theta > spotLight.cutOff)
	{
		float f = pow(((theta - spotLight.cutOff) / ( 1 - spotLight.cutOff)), spotLight.m);

		//Difusa
		c += clamp(atenuation * spotLight.intensity * Kd * dot(N, L), 0.0, 1.0) * f;

		//Especular
		vec3 V = normalize(-pos);
		vec3 R = reflect(-L, N);
		float fs = pow(max(0.0, dot(R,V)), n);
		c += atenuation * spotLight.intensity * Ks * fs * f;
	}
	
	
	return c;
}