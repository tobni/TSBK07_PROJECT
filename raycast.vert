#version 150

in vec3 inPos;

out vec3 exPos;
out vec3 enPos;

void main(void)
{

	gl_Position = transMat * vec4(inPos, 1.0);
}
