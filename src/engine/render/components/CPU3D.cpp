#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <cmath>
#include <SDL_render.h>

#include "CPU3D.h"
#include "../../engTools.h"
#include "../../logic2d.h"
#include "../../engconfig.h"

using std::endl, std::cout;

TriangleToRender::TriangleToRender(const Vertex3d& a, const Vertex3d& b, const Vertex3d& c, const Position3d& camPos, Material* mat) : v1(a), v2(b), v3(c)
{
	this->material = *mat;
	Position3d center = (a.position + b.position + c.position) / 3.0f;
	Position3d diff = center - camPos;
	distanceToCamera = diff.lengthSquared();
}

PointToRender::PointToRender(Position3d Pos, const Position3d& camPos, Material* mat)
{
	this->pos = Pos;
	this->material = *mat;
	Position3d diff = Pos - camPos;
	distanceToCamera = diff.lengthSquared();
}

CPU3D::CPU3D(SDL_Texture* screentex, SDL_Renderer* renderer, int width, int height, std::vector<uint32_t>* screenbuffer) //constructor
{
	texture = screentex;
	sdlRenderer = renderer;
	this->width = width;
	this->height = height;
	this->bufMain = screenbuffer;
	bufDepth.resize(width * height, 0);
	bufIsDrawn.resize(width * height, false);
}

void CPU3D::Clear(uint32_t color) 
{
	std::fill(bufMain->begin(), bufMain->end(), color);
	std::fill(bufDepth.begin(), bufDepth.end(), std::numeric_limits<float>::infinity()); // Unshaded background infinity away
	std::fill(bufIsDrawn.begin(), bufIsDrawn.end(), false);
}

void CPU3D::drawTri(Vertex3d& v1, Vertex3d& v2, Vertex3d& v3, Material& mat)
{
	Point2d p1(v1);
	Point2d p2(v2);
	Point2d p3(v3);

	// Cull tris behind the camera viewplane 
	if ((v1.position.cameraspace().z <= 0 && v2.position.cameraspace().z <= 0 && v3.position.cameraspace().z <= 0) ||
		(!isTriangleOnScreen(p1, p2, p3, screenwidth, screenheight)) ||
		(p1.x == -99999 || p2.x == -99999 || p3.x == -99999)
		)
	{
		return;
	}

	Position3d& loc1 = v1.position;
	Position3d& loc2 = v2.position;
	Position3d& loc3 = v3.position;

	Position3d edge1vec = loc2 - loc1;
	Position3d edge2vec = loc3 - loc1;
	Position3d normal = edge1vec.cross(edge2vec);
	normal.normalise();

	Position3d triCentre = (v1.position + v2.position + v3.position) / 3.0f;
	Position3d viewDir = (currentScene->currentCam->pos - triCentre);
	viewDir.normalise();
	uint32_t rawColour = 0xFFFF00FF; // Magenta by default
	if (mat.colour.alpha == 1.0f && mat.shadeMat) // Shade with hybrid diffuse algorithm (not physically acccurate)
	{
		Colour ucol = Colour(mat.colour.red, mat.colour.green, mat.colour.blue, mat.colour.alpha); // Copy values explicitly original material colour
		float dot = normal.dot(lightNormal);
		float value = 0.5f * dot + 0.75f;
		value = std::min(value, 1.0f);
		ucol *= value;
		rawColour = ucol.raw();
	}
	else
	{
		rawColour = mat.colour.raw(); // we can use the material colour directly
	}

	// Extract alpha from colour
	uint8_t srcA = mat.colour.alpha * 255;
	Rect2d bb(v1, v2, v3);

	int A12 = p1.y - p2.y, B12 = p2.x - p1.x, C12 = p1.x * p2.y - p2.x * p1.y;
	int A23 = p2.y - p3.y, B23 = p3.x - p2.x, C23 = p2.x * p3.y - p3.x * p2.y;
	int A31 = p3.y - p1.y, B31 = p1.x - p3.x, C31 = p3.x * p1.y - p1.x * p3.y;

	// ====== Depth shading ======
	if (false) //Condition for whether we render to bufDepth
	{
		float* pixDepth = bufDepth.data();

		for (int y = bb.min.y; y < bb.max.y; y++)
		{
			int flippedY = this->height - y;
			if (flippedY < 0 || flippedY >= this->height) continue;

			int baseIndex = flippedY * width;

			int w1 = A12 * bb.min.x + B12 * y + C12;
			int w2 = A23 * bb.min.x + B23 * y + C23;
			int w3 = A31 * bb.min.x + B31 * y + C31;

			int dw1 = A12, dw2 = A23, dw3 = A31;

			for (int x = bb.min.x; x < bb.max.x; x++)
			{
				if ((unsigned)x < (unsigned)width && w1 <= 0 && w2 <= 0 && w3 <= 0)
				{
					// calculate depth
					// easynote[20:]
					// We want to find the tested pixel in terms of a ratio between each vertex.
					// we know the distance from each vert to the camera, and can lerp these to get the tested pixel's depth.			
					float grad1 = ((float)y - p1.y) / ((float)x - p1.x); // AP->
					float grad2 = ((float)p3.y - p2.y) / ((float)p3.x - p2.x); // BC->
					float const1 = (grad1 * p1.x) - p1.y;
					float const2 = (grad2 * p2.x) - p2.y;
					int qx = (const2 - const1) / (grad2 - grad1); // The x value of the intersection point of AP-> and BC->
					Point2d q(
						qx,
						(grad1 * qx) + const1
					); // Intersection of AP and BC
					// Ratio of q along BC
					float qbybc = (float)(qx - p2.x) / p3.x;
					// p along aq
					float pbyaq = (float)(x - p1.x) / qx;
					float adepth = std::sqrt((loc1 - currentScene->currentCam->pos).lengthSquared());
					float bdepth = std::sqrt((loc2 - currentScene->currentCam->pos).lengthSquared());
					float cdepth = std::sqrt((loc3 - currentScene->currentCam->pos).lengthSquared());
					float qdepth = lerp(bdepth, cdepth, qbybc);
					float pdepth = lerp(adepth, qdepth, pbyaq);
					if (pdepth < pixDepth[baseIndex + x])
					{
						pixDepth[baseIndex + x] = pdepth;
						bufIsDrawn[baseIndex + x] = true;
					}
				}
				w1 += dw1;
				w2 += dw2;
				w3 += dw3;
			}
		}
	}
	// ======

	// Fast path: fully opaque
	if (srcA == 255) {
		//Rect2d bb(v1, v2, v3);

		//int A12 = p1.y - p2.y, B12 = p2.x - p1.x, C12 = p1.x * p2.y - p2.x * p1.y;
		//int A23 = p2.y - p3.y, B23 = p3.x - p2.x, C23 = p2.x * p3.y - p3.x * p2.y;
		//int A31 = p3.y - p1.y, B31 = p1.x - p3.x, C31 = p3.x * p1.y - p1.x * p3.y;

		uint32_t* pixShaded = bufMain->data();

		for (int y = bb.min.y; y < bb.max.y; y++)
		{
			int flippedY = this->height - y;
			if (flippedY < 0 || flippedY >= this->height) continue;

			int baseIndex = flippedY * width;

			int w1 = A12 * bb.min.x + B12 * y + C12;
			int w2 = A23 * bb.min.x + B23 * y + C23;
			int w3 = A31 * bb.min.x + B31 * y + C31;

			int dw1 = A12, dw2 = A23, dw3 = A31;

			for (int x = bb.min.x; x < bb.max.x; x++)
			{
				if ((unsigned)x < (unsigned)width && w1 <= 0 && w2 <= 0 && w3 <= 0)
				{
					pixShaded[baseIndex + x] = rawColour;
					bufIsDrawn[baseIndex + x] = true;
				}
				w1 += dw1;
				w2 += dw2;
				w3 += dw3;
			}
		}
		return;
	}

	// Fast path: fully transparent
	if (srcA == 0) {
		return;
	}
	// We copy by refernce the data of the pixbuf vector to a C-style array
	uint32_t* pixShaded = bufMain->data();

	uint8_t srcR = (rawColour >> 16) & 0xFF;
	uint8_t srcG = (rawColour >> 8) & 0xFF;
	uint8_t srcB = rawColour & 0xFF;

	for (int y = bb.min.y; y < bb.max.y; y++)
	{
		int flippedY = this->height - y;
		if (flippedY < 0 || flippedY >= this->height) continue;

		int baseIndex = flippedY * width;

		int w1 = A12 * bb.min.x + B12 * y + C12;
		int w2 = A23 * bb.min.x + B23 * y + C23;
		int w3 = A31 * bb.min.x + B31 * y + C31;

		int dw1 = A12, dw2 = A23, dw3 = A31;

		for (int x = bb.min.x; x < bb.max.x; x++)
		{
			if ((unsigned)x < (unsigned)width && w1 <= 0 && w2 <= 0 && w3 <= 0)
			{
				// We get a reference to the current pixel in the array made above; still by refernce so changes the vector
				uint32_t& dest = pixShaded[baseIndex + x];
				bufIsDrawn[baseIndex + x] = true;

				uint8_t dstR = (dest >> 16) & 0xFF;
				uint8_t dstG = (dest >> 8) & 0xFF;
				uint8_t dstB = dest & 0xFF;

				// Blend (non-premultiplied alpha)
				uint8_t outR = (srcR * srcA + dstR * (255 - srcA)) / 255;
				uint8_t outG = (srcG * srcA + dstG * (255 - srcA)) / 255;
				uint8_t outB = (srcB * srcA + dstB * (255 - srcA)) / 255;

				// Optionally blend alpha too — here we just preserve max of src/dst
				uint8_t outA = std::max(srcA, (uint8_t)(dest >> 24));

				dest = (outA << 24) | (outR << 16) | (outG << 8) | outB;
			}
			w1 += dw1;
			w2 += dw2;
			w3 += dw3;
		}
	}
}

void CPU3D::Present()
{
	SDL_UpdateTexture(texture, nullptr, bufMain->data(), width * sizeof(uint32_t));
	// Generate depth buffer visualisation

	//std::vector<uint32_t> dbgDepth;
	//dbgDepth.resize(width * height, 0xFF000000);
	//float minDepth = std::numeric_limits<float>::infinity(), maxDepth = 0;
	//for (int i = 0; i < bufDepth.size(); ++i)
	//{
	//	if (bufIsDrawn[i])
	//	{
	//		if (bufDepth[i] > maxDepth) { maxDepth = bufDepth[i]; }
	//		// Questionable optimisation to elif here but we're not gonna have 1 pixel on the screen and need a depthmap
	//		else if (bufDepth[i] < minDepth) { minDepth = bufDepth[i]; }
	//	}
	//}
	//for (int i = 0; i < bufDepth.size(); ++i)
	//{
	//	uint8_t v = 0x00;
	//	if (bufIsDrawn[i])
	//	{
	//		v = (uint8_t)(((bufDepth[i] - minDepth) / maxDepth) * 255);
	//		//std::cout << "v: " << v << '\n';
	//	}
	//	dbgDepth[i] = 0xFF000000 | (v << 16) | (v << 8) | v;
	//}

	//SDL_UpdateTexture(texture, nullptr, dbgDepth.data(), width * sizeof(uint32_t));
	SDL_RenderCopy(sdlRenderer, texture, nullptr, nullptr);
	SDL_RenderPresent(sdlRenderer);
}
