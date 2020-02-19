#include <stdint.h>
#include <glm/glm.hpp>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../../libs/stb/stb_image_write.h"

typedef uint32_t uint;
typedef glm::vec<2, double> vec2;
typedef glm::vec<3, double> vec3;
typedef glm::vec<4, double> vec4;
using glm::max;


// https://learnopengl.com/PBR/IBL/Specular-IBL
// https://github.com/Nadrin/PBR/blob/master/data/shaders/glsl/spbrdf_cs.glsl
// https://github.com/derkreature/IBLBaker/blob/master/data/shadersD3D11/IblBrdf.hlsl
// http://glslsandbox.com/e#47996.4
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html

const uint NumSamples = 1024;
const double PI = 3.141592653589793;

// Compute Van der Corput radical inverse
// See: http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
double radicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// Sample i-th point from Hammersley point set of NumSamples points total.
vec2 sampleHammersley(uint i)
{
	return vec2(double(i) / double(NumSamples), radicalInverse_VdC(i));
}

vec3 sampleGGX(vec2 Xi, vec3 N, double roughness)
{
    double a = roughness*roughness;

    double phi = 2.0 * PI * Xi.x;
    double cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    double sinTheta = sqrt(1.0 - cosTheta*cosTheta);

    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // from tangent-space vector to world-space sample vector
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

double GeometrySchlickGGX(double NdotV, double roughness)
{
    double a = roughness;
    double k = (a * a) / 2.0;

    double nom   = NdotV;
    double denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
double GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    double NdotV = max(dot(N, V), 0.0);
    double NdotL = max(dot(N, L), 0.0);
    double ggx2 = GeometrySchlickGGX(NdotV, roughness);
    double ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}



vec2 IBL_BRDF(double x, double y)
{
	// Get integration parameters.
	double NdotV = x;
	double roughness = y;

    vec3 V;
    V.x = sqrt(1.0 - NdotV * NdotV);
    V.y = 0.0;
    V.z = NdotV;

	// We will now pre-integrate Cook-Torrance BRDF for a solid white environment and save results into a 2D LUT.
	// DFG1 & DFG2 are terms of split-sum approximation of the reflectance integral.
	// For derivation see: "Moving Frostbite to Physically Based Rendering 3.0", SIGGRAPH 2014, section 4.9.2.
	double DFG1 = 0;
	double DFG2 = 0;

    vec3 N = vec3(0.0, 0.0, 1.0);

	for(uint i=0; i<NumSamples; ++i)
	{
		vec2 Xi = sampleHammersley(i);
        vec3 H  = sampleGGX(Xi, N, roughness);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        double NdotL = max(L.z, 0.0);
        double NdotH = max(H.z, 0.0);
        double VdotH = max(dot(V, H), 0.0);

		if(NdotL > 0.0)
		{
            float G = GeometrySmith(N, V, L, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);

			DFG1 += (1 - Fc) * G_Vis;
			DFG2 += Fc * G_Vis;
		}
	}

	return vec2(DFG1, DFG2) / double(NumSamples);
}


int main()
{
	int size = 1024;
	uint8_t* lut = new uint8_t[size * size * 3];

	for (int x = 0; x < size; ++x)
	{
		for (int y = 0; y < size; ++y)
		{
			double fx = double(x) + 0.5;
			fx /= size;

			double fy = double(size - 1 - y) + 0.5;
			fy /= size;

			vec2 p = IBL_BRDF(fx, fy);
			lut[3 * (x + y * size) + 0] = p.x * 255.0;
			lut[3 * (x + y * size) + 1] = p.y * 255.0;
		}
	}

	stbi_write_png("ibl_brdf.png", size, size, 3, lut, 0);

	return 0;
}
