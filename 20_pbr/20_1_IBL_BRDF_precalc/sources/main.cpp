#include <stdint.h>
#include <glm/glm.hpp>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../../libs/stb/stb_image_write.h"

typedef uint32_t uint;
typedef glm::vec<2, double> vec2;
typedef glm::vec<3, double> vec3;
typedef glm::vec<4, double> vec4;
using glm::max;


// https://github.com/Nadrin/PBR/blob/master/data/shaders/glsl/spbrdf_cs.glsl
// https://github.com/derkreature/IBLBaker/blob/master/data/shadersD3D11/IblBrdf.hlsl
// http://glslsandbox.com/e#47996.4
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html

const uint NumSamples = 1024;
const double InvNumSamples = 1.0 / double(NumSamples);

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
	return vec2(i * InvNumSamples, radicalInverse_VdC(i));
}


// Importance sample GGX normal distribution function for a fixed roughness value.
// This returns normalized half-vector between Li & Lo.
// For derivation see: http://blog.tobias-franke.eu/2014/03/30/notes_on_importance_sampling.html
vec3 sampleGGX(double u1, double u2, double roughness)
{
	double alpha = roughness * roughness;
	double phi = 2.0f * PI * u1;

	double cosTheta = sqrt((1.0 - u2) / (1.0 + (alpha*alpha - 1.0) * u2));
	double sinTheta = sqrt(1.0 - cosTheta*cosTheta); // Trig. identity

	// Convert to Cartesian upon return.
	return vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

// Single term for separable Schlick-GGX below.
double gaSchlickG1(double cosTheta, double k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method (IBL version).
double gaSchlickGGX_IBL(double cosLi, double cosLo, double roughness)
{
	double r = roughness;
	double k = (r * r) / 2.0; // Epic suggests using this roughness remapping for IBL lighting.
	return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}


vec2 IBL_BRDF(double x, double y)
{
	// Get integration parameters.
	double cosLo = x;
	double roughness = y;

	// Make sure viewing angle is non-zero to avoid divisions by zero (and subsequently NaNs).
	// cosLo = max(cosLo, Epsilon);

	// Derive tangent-space viewing vector from angle to normal (pointing towards +Z in this reference frame).
	vec3 Lo = vec3(sqrt(1.0 - cosLo*cosLo), 0.0, cosLo);

	// We will now pre-integrate Cook-Torrance BRDF for a solid white environment and save results into a 2D LUT.
	// DFG1 & DFG2 are terms of split-sum approximation of the reflectance integral.
	// For derivation see: "Moving Frostbite to Physically Based Rendering 3.0", SIGGRAPH 2014, section 4.9.2.
	double DFG1 = 0;
	double DFG2 = 0;

	for(uint i=0; i<NumSamples; ++i) {
		vec2 u  = sampleHammersley(i);

		// Sample directly in tangent/shading space since we don't care about reference frame as long as it's consistent.
		vec3 Lh = sampleGGX(u.x, u.y, roughness);

		// Compute incident direction (Li) by reflecting viewing direction (Lo) around half-vector (Lh).
		vec3 Li = 2.0f * dot(Lo, Lh) * Lh - Lo;

		double cosLi   = Li.z;
		double cosLh   = Lh.z;
		double cosLoLh = max(dot(Lo, Lh), 0.0);

		if(cosLi > 0.0) {
			double G  = gaSchlickGGX_IBL(cosLi, cosLo, roughness);
			double Gv = G * cosLoLh / (cosLh * cosLo);
			double Fc = pow(1.0 - cosLoLh, 5);

			DFG1 += (1 - Fc) * Gv;
			DFG2 += Fc * Gv;
		}
	}

	return vec2(DFG1, DFG2) * InvNumSamples;
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
