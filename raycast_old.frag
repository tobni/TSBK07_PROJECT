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

// bounding cube
uniform vec3[8]		cubeVert;
uniform int[36]		cubeInd;

bool inVolume = false;
// Helper boolean to visualize use of bounding cube
bool usingPlane = false;

// Function for readability. This is where coordinates are rotated, THEN accessed.
float textureAccess(vec3 voxelCoord, vec3 offset)
{
	return texture(texVol, vec3(rotMat * vec4(voxelCoord + stepSize*offset, 1.0))).x;
}

// Function to test ray-intersections with bounding-box of triangle.
vec3 intersectionPoint(	vec3 _a, vec3 _b, vec3 _c, 
						vec3 rayDir, vec3 quadCoord)
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
		inVolume = true;
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
		return quadCoord;
	}
}


void main(void)
{
	// Final values from ray integral
	float intensity = 0;
	float alpha = 0;

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

	vec3 voxelCoord = enPoint;

	int steps = int(length(enPoint - exPoint) /stepSize);

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
		
		// Front-to-back composite acquisition along the ray for color
		intensity += (1.0 - alpha) * intensitySample * alphaSample;

		alpha += alphaSample;

		// Stop ray if opacity is full
		if(alpha >= 1.0) 
		{
			alpha = 1.0;
			break; 
		}
		// Advance the ray
		voxelCoord += rayStep;
	}

	outColor = vec4(intensity, intensity, intensity, alpha);
}
