#version 150

in vec2 texCoord;

out vec4 outColor;

uniform sampler3D	texVol;
uniform float		stepSize;
uniform float		focalLength;
uniform float		distance;
uniform float		alphaScale;
uniform mat4		rotMat;

float textureAccess(vec3 voxelCoord, vec3 offset)
{
	return texture(texVol, vec3(rotMat * vec4(voxelCoord + stepSize*offset, 1.0))).x;
}


vec3 sobel3D(vec3 voxelCoord)
{
	vec3 gradient = vec3(0, 0, 0);
	mat3 sobelX_left = mat3(1, 0, -1,
							2, 0, -2,
							1, 0, -1);
	mat3 sobelX_middle = 2 * sobelX_left;
	mat3 sobelX_right = sobelX_left;

	mat3 sobelY_left = mat3(-1, -2, -1,
							0, 0, 0,
							1, 2, 1);

	mat3 sobelY_middle = 2 * sobelY_left;

	mat3 sobelY_right = sobelY_left;

	mat3 sobelZ_left = mat3(1, 2, 1,
							2, 4, 2,
							1, 2, 1);
	mat3 sobelZ_right = -1.0 * sobelZ_left;

	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			for(int k = 0; k < 3; k++)
			{
				if (i == 0) {
				gradient.xzy += vec3(sobelX_left[j][k], sobelY_left[j][k], sobelZ_left[j][k]) * textureAccess(voxelCoord, vec3(k-1,i-1,j-1));
				}
				if (i == 1) {
				gradient.xz += vec2(sobelX_middle[j][k], sobelY_middle[j][k]) * textureAccess(voxelCoord, vec3(k-1,i-1,j-1));
				}
				if (i == 2) {
				gradient.xzy += vec3(sobelX_right[j][k], sobelY_right[j][k], sobelZ_right[j][k]) * textureAccess(voxelCoord, vec3(k-1,i-1,j-1));
				}
			}
	
		}
		
	}

	return gradient;
}

void main(void)
{
	int steps = int(1.0 / stepSize);
	float intensity = 0;
	float alpha = 0;

	vec3 prp = vec3(0.5, 0.5, -focalLength);
	vec3 rayDir = vec3(texCoord, 0.0) - prp;
	vec3 rayStep = normalize(rayDir)*stepSize;
	vec3 voxelCoord = vec3(texCoord, distance);
	vec3 light = vec3(0.53,0.53,0.53);

	float intensitySample, alphaSample, lightSample;
	vec3 normal = vec3(0,0,0), gradient = vec3(0,0,0);
	for(int i = 0; i < steps; i++)
	{
		intensitySample = textureAccess(voxelCoord, vec3(0,0,0));
		
		if (intensitySample > 0.18) {
		alphaSample = intensitySample * alphaScale;

		intensity += (1.0 - alpha) * intensitySample * alphaSample;
		gradient += (1.0 - alpha) * sobel3D(voxelCoord) * alphaSample;

		alpha += alphaSample;

		}
		if(alpha >= 1.0) 
		{	
			alpha = 1.0;
			break; 
		}
		

		voxelCoord += rayStep;
	}
	normal = normalize(gradient);
	
	intensity = 0.8 * intensity + 0.9 * intensity * clamp(dot(normal, light), 0.0, 1.0) + 0.09 * 
	pow(max(0.0, dot(reflect(-light, normal), rayDir)), 
                 2);

	outColor = vec4(intensity, intensity, intensity, alpha);
}
