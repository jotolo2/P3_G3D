#version 330 core

out vec4 outColor;
uniform vec3 Id;

void main()
{
	//El color del cubo que representa la luz tendra la intensidad difusa que emite
	outColor = vec4(Id, 1.0);   
}

