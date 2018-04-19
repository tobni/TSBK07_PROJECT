#version 150


in vec3 inPos;
in vec3 exPos;

out vec3 outColor;

uniform sampler3D volTex;

void main(void)
{
	outColor = vec4(1.0, 1.0, 0.8, 1.0);
}
