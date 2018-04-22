#version 150

in vec3 inPos;
in vec2 inTexCoord;

out vec2 texCoord;

void main(void)
{
	texCoord = inTexCoord;
	gl_Position = vec4(inPos, 1.0);
}
