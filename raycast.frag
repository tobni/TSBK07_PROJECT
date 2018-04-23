#version 150

in vec2 texCoord;

out vec4 outColor;

uniform sampler3D	texVol;
uniform float		stepSize;
uniform float		focalLength;
uniform float		alphaScale;
uniform mat4		rotMat;

void main(void)
{
	int steps = int(1.0 / stepSize);
	
	float intensity = 0;
	float alpha = 0;
	

	vec3 prp = vec3(0.5, 0.5, focalLength);
	vec3 rayDir = vec3(texCoord, 0.0) - prp;
	vec3 rayStep = normalize(rayDir)*stepSize;
	vec3 voxelCoord = vec3(texCoord, focalLength);

	float intensitySample, alphaSample;	

	for(int i = 0; i < steps; i++)
	{
		intensitySample = texture(texVol, vec3(rotMat * vec4(voxelCoord, 1.0))).x;
		alphaSample = intensitySample * alphaScale;
		
		intensity += (1.0 - alpha) * intensitySample * alphaSample;
		alpha += alphaSample;

		if(alpha >= 1.0) 
		{
			alpha = 1.0;
			break; 
		}

		voxelCoord += rayStep;
	}

	outColor = vec4(intensity, intensity, intensity, alpha);
}
