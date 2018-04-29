#version 150

in vec2 texCoord;

out vec4 outColor;

uniform sampler3D	texVol;
uniform float		stepSize;
uniform float		focalLength;
uniform float		distance;
uniform float		alphaScale;
uniform mat4		rotMat;
uniform mat4		mdlMat;

// Sobel 3D Filter Kernels, flipped. 
mat3 sobelX_left = mat3(1, 0, -1,
						2, 0, -2,
						1, 0, -1);
mat3 sobelX_middle = 2 * sobelX_left;
mat3 sobelX_right = sobelX_left;

mat3 sobelY_left = mat3(1, 2, 1,
						0, 0, 0,
						-1, -2, -1);
mat3 sobelY_middle = 2 * sobelY_left;
mat3 sobelY_right = sobelY_left;

mat3 sobelZ_left = mat3(1, 2, 1,
						2, 4, 2,
						1, 2, 1);
// sobelZ_middle is the 3x3 0-matrix.
mat3 sobelZ_right = -1.0 * sobelZ_left;

// Helper boolean to visualize use of bounding cube
bool usingPlane = false;

// Function for readability. This is where coordinates are rotated, THEN accessed.
float textureAccess(vec3 voxelCoord, vec3 offset)
{
	return texture(texVol, vec3(rotMat * vec4(voxelCoord + stepSize*offset, 1.0))).x;
}

// NOT DONE. Function to test ray-intersections with bounding-box of triangle.
vec3 intersectionPoint(	vec3 _a, vec3 _b, vec3 _c, 
						vec3 rayDir, vec3 quadCoord, 
						float distance)
{	vec3 enPoint = vec3(0,0,0);
	vec3 a = vec3(mdlMat * vec4(_a, 1));
	vec3 b = vec3(mdlMat * vec4(_b, 1));
	vec3 c = vec3(mdlMat * vec4(_c, 1));
	vec3 planeNormal = cross(b-a, c-a); 
	float D = dot(planeNormal, a);
	float t = (dot(a - quadCoord, planeNormal)) / dot(planeNormal, rayDir); 
	enPoint = quadCoord + t * rayDir;
	if (dot(planeNormal, cross(b-a, enPoint - a)) > 0 && 
		dot(planeNormal, cross(c-b, enPoint - b)) > 0 && 
		dot(planeNormal, cross(a-c, enPoint - c)) > 0) 
	{
		if (t < 0) {
		return quadCoord;
		}
		else {
		usingPlane = true;
		return enPoint;
		}
	}
	else
	{
		//discard;
		return quadCoord;
	}
}


// Applies the filter kernels. The function computes correlation, not convolution.
// Kernels are flipped (see above).
vec3 sobel3D(vec3 voxelCoord)
{
	vec3 gradient = vec3(0, 0, 0);

	for(int i = 0; i < 3; i++) {
		for(int j = 0; j < 3; j++) {
			for(int k = 0; k < 3; k++) {
				if (i == 0) {
				gradient.xyz += vec3(sobelX_left[j][k], sobelY_left[j][k], sobelZ_left[j][k]) * textureAccess(voxelCoord, vec3(k-1,j-1,i-1));
				}
				if (i == 1) {
				gradient.xy += vec2(sobelX_middle[j][k], sobelY_middle[j][k]) * textureAccess(voxelCoord, vec3(k-1,j-1,i-1));
				}
				if (i == 2) {
				gradient.xyz += vec3(sobelX_right[j][k], sobelY_right[j][k], sobelZ_right[j][k]) * textureAccess(voxelCoord, vec3(k-1,j-1,i-1));
				}
			}
		}
	}

	return gradient;
}

void main(void)
{
	int steps = int(1.0 / stepSize);

	// Final values from ray integral
	float intensity = 0;
	float alpha = 0;
	vec3 gradient = vec3(0,0,0);

	// Ray definitions
	vec3 prp = vec3(0.5, 0.5, -focalLength + distance);
	vec3 rayDir = normalize( vec3(texCoord, distance) - prp);
	vec3 rayStep = rayDir*stepSize;

	// Alternate these for use of bounding cube 
	vec3 voxelCoord = vec3(texCoord, distance);
	/*vec3 voxelCoord = intersectionPoint(vec3(1,0,0), vec3(1,1,0),vec3(0,1,0), 
										rayDir, vec3(texCoord, distance), distance);*/
	
	// Hardcoded light source, negative Z due to prp definition (it's "behind" the screen, negative z)
	vec3 light = vec3(0.5,0.53,-0.83);

	// Data gathered from the volume for each step
	float intensitySample, alphaSample;

	for(int i = 0; i < steps; i++)
	{
		intensitySample = textureAccess(voxelCoord, vec3(0,0,0));
		
		// toss samples that are basically black
		if (intensitySample > 0.00) { 
		// Alpha is scaled to intensity (kinda boring, x-ray-esque)
		alphaSample = intensitySample * alphaScale;

		// Front-to-back composite acquisition along the ray for gradient and color
		intensity += (1.0 - alpha) * intensitySample * alphaSample;
		gradient += (1.0 - alpha) * sobel3D(voxelCoord) * alphaSample;

		alpha += alphaSample;

		}
		// Stop ray if opacity is full
		if (alpha >= 1.0) 
		{	
			alpha = 1.0;
			break; 
		}
		// Stop ray if it has penetrated entire volume
		if (voxelCoord.z > 1.0) {
			break;
		}
		
		// Advance the ray
		voxelCoord += rayStep;
	}
	vec3 normal = normalize(gradient);

	// Phong shading from ray(eye)-direction and normal
	float ambient = intensity;
	float diffuse = intensity * clamp(dot(normal, light), 0.0, 1.0);
	float specular = intensity * 0.2 * pow(max(0.0, dot(reflect(light, normal), rayDir)),3);
	vec4 color = vec4(1.0, 1.0, 1.0, 0) * (ambient + diffuse + specular) + vec4(0,0,0,alpha);

	/*if (usingPlane == true)
		color = vec4(0.0, 0.0, 1.0, 1.0);*/ // This is to see the bounding cube

	outColor = vec4(color);
}
