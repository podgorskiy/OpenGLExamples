#include "Texture.h"
#include <GL/gl3w.h>
#include <stdio.h>
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>
#include <string.h>


Texture::Texture() :m_textureHandle(-1)
{
	glGenTextures(1, &m_textureHandle);
}


void Texture::Bind(int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_textureHandle);
}

void Texture::BindCube(int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureHandle);
}

void Texture::UnBind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture()
{
	if (m_textureHandle != -1) {
		glDeleteTextures(1, &m_textureHandle);
		m_textureHandle = -1;
	}
}

TexturePtr Texture::LoadTexture(const std::string& path)
{
	Texture* texture = new Texture;

	FILE* f = fopen(path.c_str(), "rb");

	if (f == nullptr)
	{
		return TexturePtr();
	}

	Texture::TextureHeader& header = texture->header;
	int dummy;
	int version;
	int flags;
	int channelType;
	int depth;
	int numSurfaces;
	int numFaces;
	int metaDataSize;
	int pixelFormatCompressed;
	int colourSpace;

	int fsize;

	fseek(f, 0L, SEEK_END);
	fsize = ftell(f);
	fseek(f, 0L, SEEK_SET);

	fread(&version, 4, 1, f);
	fread(&flags, 4, 1, f);
	fread(&pixelFormatCompressed, 4, 1, f);
	fread(&header.pixelFormatUncompressed, 4, 1, f);
	fread(&colourSpace, 4, 1, f);
	fread(&channelType, 4, 1, f);
	fread(&header.height, 4, 1, f);
	fread(&header.width, 4, 1, f);
	fread(&depth, 4, 1, f);
	fread(&numSurfaces, 4, 1, f);
	fread(&numFaces, 4, 1, f);
	fread(&header.MIPMapCount, 4, 1, f);
	fread(&metaDataSize, 4, 1, f);

	header.pixelFormatCompressed = static_cast<Texture::EPVRTPixelFormat>(pixelFormatCompressed);
	header.colourSpace = static_cast<Texture::EPVRTColourSpace>(colourSpace);

	if (version != 0x03525650)
	{
		printf("Error reading texture\n");
		return TexturePtr();
	}

	assert(version == 0x03525650);
	assert(flags == 0 || flags == 2);
	assert(depth == 1);
	assert(header.colourSpace == 1 || header.colourSpace == 0);
	//assert(channelType == 0);
	assert(numSurfaces == 1);
	assert(numFaces == 1 || numFaces == 6);

	header.cubemap = numFaces == 6;

	fseek(f, ftell(f) + metaDataSize, SEEK_SET);

	if (header.cubemap)
	{
		texture->BindCube(0);
	}
	else
	{
		texture->Bind(0);
	}

	char* tempBuffer = NULL;
	char* tempBuffer2 = NULL;
	char* p = NULL;

	{
		int size = fsize - ftell(f);
		tempBuffer = new char[size];
		tempBuffer2 = new char[header.width * header.height * 4];
		fread(tempBuffer, size, 1, f);
		p = tempBuffer;
	}

	int mipmapDivider = 1;
	for (int mipmap = 0; mipmap < header.MIPMapCount; ++mipmap)
	{
		for (int face = 0; face < numFaces; ++face)
		{
			int width = header.width / mipmapDivider;
			int height = header.height / mipmapDivider;
			int size = (width * height) * 4;
			int faceType = header.cubemap ? (GL_TEXTURE_CUBE_MAP_POSITIVE_X + face) : GL_TEXTURE_2D;

			for(int r = 0; r < height; ++r)
			{
					memcpy(tempBuffer2 + 4 * (width * r), p + 4 * (width * (height - 1 - r)), 4 * width);
			}

			glTexImage2D(faceType, mipmap, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tempBuffer2);
			p += size;
		}
		mipmapDivider *= 2;
	}

	int faceType = header.cubemap ? (GL_TEXTURE_CUBE_MAP) : GL_TEXTURE_2D;

	if (header.MIPMapCount > 1)
	{
		glTexParameteri(faceType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}
	else
	{
		glTexParameteri(faceType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	glTexParameteri(faceType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	texture->UnBind();

	delete tempBuffer;
	return TexturePtr(texture);
}
