#include "CPURenderer.h"
#include "../engconfig.h"
#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>

using std::endl, std::cout;

TriangleToRender::TriangleToRender(const Vertex3d& a, const Vertex3d& b, const Vertex3d& c, const Position3d& camPos, Material* mat) : v1(a), v2(b), v3(c)
{
	this->material = *mat;
	Position3d center = (a.position + b.position + c.position) / 3.0f;
	Position3d diff = center - camPos;
	distanceToCamera = diff.lengthSquared();
}


CPURenderer::CPURenderer(SDL_Renderer* renderer, int w, int h): sdlRenderer(renderer), width(w), height(h)
{
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
	pixelBuffer.resize(width * height, 0xFF000000); // opaque black
}

CPURenderer::~CPURenderer() 
{
	SDL_DestroyTexture(texture);
}

void CPURenderer::Clear(uint32_t color) 
{
	std::fill(pixelBuffer.begin(), pixelBuffer.end(), color);
}

inline void CPURenderer::SetPixel(int x, int y, uint32_t color)
{
	int screenY = screenheight - y;
	if ((unsigned)x >= (unsigned)width || (unsigned)screenY >= (unsigned)height)
		return;

	uint32_t& dest = pixelBuffer[screenY * width + x];

	uint8_t srcA = color >> 24;
	if (srcA == 255) {
		// Fully opaque: just write
		dest = color;
		return;
	}
	if (srcA == 0) {
		// Fully transparent: do nothing
		return;
	}
	cout << "transparent" << endl;
	// Extract source color components
	uint8_t srcR = (color >> 16) & 0xFF;
	uint8_t srcG = (color >> 8) & 0xFF;
	uint8_t srcB = color & 0xFF;

	// Extract destination color components
	uint8_t dstR = (dest >> 16) & 0xFF;
	uint8_t dstG = (dest >> 8) & 0xFF;
	uint8_t dstB = dest & 0xFF;

	// Blend (non-premultiplied alpha)
	uint8_t outR = (srcR * srcA + dstR * (255 - srcA)) / 255;
	uint8_t outG = (srcG * srcA + dstG * (255 - srcA)) / 255;
	uint8_t outB = (srcB * srcA + dstB * (255 - srcA)) / 255;

	// Optionally blend alpha too — here we just preserve max of src/dst
	uint8_t outA = std::max(srcA, (uint8_t)(dest >> 24));

	// Repack
	dest = (outA << 24) | (outR << 16) | (outG << 8) | outB;
}

void CPURenderer::drawScene(Scene& scene)
{
	std::vector<TriangleToRender> triangles;

	if (!scene.currentCam) return;
	Position3d camPos = scene.currentCam->pos;

	for (Mesh& mesh : scene.meshes)
	{
		Position3d pos = mesh.position;
		Rotation3d rot = mesh.rotation;
		for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3)
		{
			Vertex3d& v1 = mesh.vertices[mesh.indices[i]];
			Vertex3d& v2 = mesh.vertices[mesh.indices[i + 1]];
			Vertex3d& v3 = mesh.vertices[mesh.indices[i + 2]];
			//Material mat = mesh.materials[mesh.matIndices[i / 3]];
			Material mat = mesh.materials[0];
			triangles.emplace_back(v1, v2, v3, camPos, &mat);
		}
	}

	std::sort(triangles.begin(), triangles.end(),
		[](const TriangleToRender& a, const TriangleToRender& b) {
			return a.distanceToCamera > b.distanceToCamera;
		});

	// Draw
	for (TriangleToRender& tri : triangles)
	{
		drawTri(tri.v1, tri.v2, tri.v3, tri.material);
	}
}

void CPURenderer::drawTri(Vertex3d& v1, Vertex3d& v2, Vertex3d& v3, Material& mat)
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
    if (mat.colour.alpha == 1.0f)
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
		rawColour = mat.colour.raw(); // Because it's volumetric, we can use the material colour directly
    }

    // Extract alpha from colour
	uint8_t srcA = mat.colour.alpha * 255;

    // Fast path: fully opaque
    if (srcA == 255) {
        Rect2d bb(v1, v2, v3);

        int A12 = p1.y - p2.y, B12 = p2.x - p1.x, C12 = p1.x * p2.y - p2.x * p1.y;
        int A23 = p2.y - p3.y, B23 = p3.x - p2.x, C23 = p2.x * p3.y - p3.x * p2.y;
        int A31 = p3.y - p1.y, B31 = p1.x - p3.x, C31 = p3.x * p1.y - p1.x * p3.y;

        uint32_t* pixels = pixelBuffer.data();

        for (int y = bb.min.y; y < bb.max.y; y++)
        {
            int flippedY = screenheight - y;
            if (flippedY < 0 || flippedY >= screenheight) continue;

            int baseIndex = flippedY * width;

            int w1 = A12 * bb.min.x + B12 * y + C12;
            int w2 = A23 * bb.min.x + B23 * y + C23;
            int w3 = A31 * bb.min.x + B31 * y + C31;

            int dw1 = A12, dw2 = A23, dw3 = A31;

            for (int x = bb.min.x; x < bb.max.x; x++)
            {
                if ((unsigned)x < (unsigned)width && w1 <= 0 && w2 <= 0 && w3 <= 0)
                {
                    pixels[baseIndex + x] = rawColour;
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

    // Blending path: partially transparent
    Rect2d bb(v1, v2, v3);

    int A12 = p1.y - p2.y, B12 = p2.x - p1.x, C12 = p1.x * p2.y - p2.x * p1.y;
    int A23 = p2.y - p3.y, B23 = p3.x - p2.x, C23 = p2.x * p3.y - p3.x * p2.y;
    int A31 = p3.y - p1.y, B31 = p1.x - p3.x, C31 = p3.x * p1.y - p1.x * p3.y;

    uint32_t* pixels = pixelBuffer.data();

    uint8_t srcR = (rawColour >> 16) & 0xFF;
    uint8_t srcG = (rawColour >> 8) & 0xFF;
    uint8_t srcB = rawColour & 0xFF;

    for (int y = bb.min.y; y < bb.max.y; y++)
    {
        int flippedY = screenheight - y;
        if (flippedY < 0 || flippedY >= screenheight) continue;

        int baseIndex = flippedY * width;

        int w1 = A12 * bb.min.x + B12 * y + C12;
        int w2 = A23 * bb.min.x + B23 * y + C23;
        int w3 = A31 * bb.min.x + B31 * y + C31;

        int dw1 = A12, dw2 = A23, dw3 = A31;

        for (int x = bb.min.x; x < bb.max.x; x++)
        {
            if ((unsigned)x < (unsigned)width && w1 <= 0 && w2 <= 0 && w3 <= 0)
            {
                uint32_t& dest = pixels[baseIndex + x];

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

void CPURenderer::Present()
{
	SDL_UpdateTexture(texture, nullptr, pixelBuffer.data(), width * sizeof(uint32_t));
	SDL_RenderCopy(sdlRenderer, texture, nullptr, nullptr);
	SDL_RenderPresent(sdlRenderer);
}
