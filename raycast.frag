#version 150

in vec2 texCoord;

out vec4 outColor;

uniform sampler3D	texVol;
uniform float		stepSize;

void main(void)
{
	int steps = int(1.0 / stepSize);
	
	float intensity = 0;
	float alpha = 0;
	
	vec3 rayDir = vec3(0.0, 0.0, 1.0);
	vec3 rayStep = normalize(rayDir)*stepSize;
	vec3 voxelCoord = vec3(texCoord, 0.0); // Start position hardcoded

	for(int i = 0; i < steps; i++)
	{
		intensity += (1.0 - alpha) * texture(texVol, voxelCoord).x * stepSize;
		alpha += (1.0 - alpha) * stepSize;
		voxelCoord += rayStep;
	}

	outColor = vec4(intensity, intensity, intensity, alpha);
}
