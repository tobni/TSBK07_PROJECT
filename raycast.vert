#version 150

in vec3 inPos;
in inTexCoord;

out vec2 exTexCoord;
out vec3 exPos;

//uniform mat4 transMat;

void main(void)
{
	gl_Position = vec4(inPos, 1.0);
}
