#version 150

in vec2 texCoord;

out vec4 outColor;

uniform	sampler3D	texVol;
uniform	float		stepSize;
uniform	float		focalLength;
uniform	float		distance;
uniform	float		alphaScale;
uniform	mat4		rotMat;
uniform	mat4		mdlMat;

// bounding cube
uniform vec3[8]		cubeVert;
uniform int[36]		cubeInd;

// Pseudo-random number generator
float rand(vec2 st) {
    return fract(sin(dot(st.xy,vec2(12.9898,78.233)))*343223.0123);
}


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

bool inVolume = false;

// Function for readability. This is where coordinates are rotated, THEN accessed.
float textureAccess(vec3 voxelCoord, vec3 offset)
{
	return texture(texVol, vec3(rotMat * vec4(voxelCoord + stepSize*offset, 1.0))).x;
}

// Function to test ray-intersections with triangle.
vec3 intersectionPoint(	vec3 _a, vec3 _b, vec3 _c, 
						vec3 rayDir, vec3 quadCoord)
{	
	vec3 a = vec3(mdlMat * vec4(_a, 1));
	vec3 b = vec3(mdlMat * vec4(_b, 1));
	vec3 c = vec3(mdlMat * vec4(_c, 1));
	vec3 planeNormal = cross(b-a, c-a); 
	float D = dot(planeNormal, a);
	float t = (dot(a - quadCoord, planeNormal)) / dot(planeNormal, rayDir); 
	vec3 point = quadCoord + t * rayDir;
	if	(dot(planeNormal, cross(b-a, point - a)) > 0 && 
		dot(planeNormal, cross(c-b, point - b)) > 0 && 
		dot(planeNormal, cross(a-c, point - c)) > 0) 
	{
		inVolume = true;
		if (t < 0) {
			return quadCoord;
		}
		else {
			return point;
		}
		
	}
	else {
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
	// Final values from ray integral
	float intensity = 0;
	float alpha = 0;
	float phong = 0;
	vec3 gradient = vec3(0,0,0), normal = vec3(0,0,0);

	// Ray definitions
	vec3 prp = vec3(0.5, 0.5, -focalLength + distance);
	vec3 quadCoord = vec3(texCoord, distance);
	vec3 rayDir = normalize(quadCoord - prp);
	vec3 rayStep = rayDir*stepSize;

	// Bounding cube test variables
	vec3 tempPoint, exPoint = quadCoord, enPoint = quadCoord;
	vec3 a, b, c;
	bool first = true;

	for (int triangle = 0; triangle < 12; triangle++)
	{
		a = cubeVert[cubeInd[triangle*3]];
		b = cubeVert[cubeInd[triangle*3+1]];
		c = cubeVert[cubeInd[triangle*3+2]];
		tempPoint = intersectionPoint(a, b, c, rayDir, quadCoord);
		if (tempPoint == quadCoord) {
			continue;
		}
		// First guess on what is the exit point of the ray
		if (first == true)
		{
			exPoint = tempPoint;
			first = false;
			continue;
		}
		// Check if the guess was correct, swap coordinates if we weren't
		if (length(tempPoint - quadCoord) > length(exPoint - quadCoord) )
		{
			enPoint = exPoint;
			exPoint = tempPoint;
			break;
		}
		else
		{
			enPoint = tempPoint;
			break;
		}

	}

	if (inVolume == false) { 
		discard; 
	}

	// Pseudo-random ray-offset to reduce ringing
	vec3 voxelCoord = enPoint + rayStep*(rand(enPoint.xy));

	int steps = int(length(enPoint - exPoint)/stepSize);
	if (steps < 30)
	{
		discard;
	}

	// Hardcoded light source, negative Z due to prp definition (it's "behind" the screen, negative z)
	vec3 light = normalize(vec3(0.5,0.53,-0.83));

	// Data gathered from the volume for each step
	float intensitySample, alphaSample;

	for(int i = 0; i < steps; i++)
	{
		intensitySample = textureAccess(voxelCoord, vec3(0,0,0));
		
		// toss samples that are basically black
		if (intensitySample < 0.18) { 
			voxelCoord += rayStep;
			continue;
		}
		// Alpha is scaled to intensity (kinda boring, x-ray-esque)
		alphaSample = intensitySample * alphaScale;

		// Front-to-back compositing acquisition along the ray for color and opacity
		gradient = sobel3D(voxelCoord);

		// Phong shading from ray(eye)-direction and normal
		normal = normalize(gradient);
		phong = (1 + clamp(dot(normal, light), 0.0, 1.0) + 0.2 * pow(max(0.0, dot(reflect(light, normal), rayDir)),3));
		
		intensity += (1.0 - alpha) * intensitySample * alphaSample * phong;

		alpha += alphaSample;

		// Stop ray if opacity is full
		if (alpha >= 1.0) 
		{	
			alpha = 1.0;
			break; 
		}
		// Advance the ray
		voxelCoord += rayStep;
	}

	outColor = vec4(intensity, intensity, intensity, alpha);
}
