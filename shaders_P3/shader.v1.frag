#version 330 core

out vec4 outColor;

in vec3 color;
in vec3 pos;
in vec3 norm;
in vec2 texCoord;

in vec3 Np;
in vec3 Tp;

uniform sampler2D colorTex;
uniform sampler2D emiTex;
uniform sampler2D normalTex;
uniform sampler2D specularTex;

uniform mat4 modelView;

//Propiedades de la luz puntual
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

vec3 shade();

void main()
{
	Ka = texture(colorTex, texCoord).rgb;
	Kd = texture(colorTex, texCoord).rgb;
	Ke = texture(emiTex, texCoord).rgb;
	Ks = texture(specularTex, texCoord).rgb;

	//OPCIONAL 2.f
	//IMPLEMENTACIÓN DEL BUMP MAPPING
	N = normalize(Np);
	vec3 T = normalize(Tp);
	vec3 B = normalize(cross(N, T));
	mat3 TBN = transpose(mat3(T, B, N));

	vec3 Nbump = ((texture(normalTex, texCoord).rgb) * 2 - 1);
	vec3 Nv = (modelView * vec4(normalize(Nbump * TBN), 0.0)).xyz;
	N = normalize(Nv);

	outColor = vec4(shade(), 1.0);   
}

vec3 shade()
{
	//Ambiental
	vec3 c = Ia * Ka;

	//Emisiva
	c += Ke;

	//Luz puntual
	vec3 lpos = lightPos.xyz;
	vec3 L =  lpos - pos;

	//OPCIONAL 2.e
	//ATENUACIÓN CON LA DISTANCIA
	float distance = length(L);
	float atenuation = min(1 / (spotLight.constant + spotLight.linear * distance + spotLight.quadratic * distance * distance), 1);
	L = normalize(L);

	//Difusa
	vec3 diffuse = atenuation * Id * Kd * dot (L,N);
	c += clamp(diffuse, 0.0, 1.0);
	
	//Especular
	vec3 V = normalize (-pos);
	vec3 R = normalize (reflect (-L,N));
	float factor = max(dot(R,V), 0.0);
	vec3 specular = atenuation * Is * Ks * pow(factor, n);
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
