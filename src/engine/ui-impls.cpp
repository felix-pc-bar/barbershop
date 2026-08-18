#include "ui-impls.h"
#include "render/components/bmpfont.h"
#include "render/render.h"
#include "ui.h"
#include <memory>

Console::Console(cRenderer* renderer, bmpFont* font)
{
	renderer->UI->children.emplace_back(std::make_unique<LayoutElement>("Console", frac2d{0.5f, 0.0f}, frac2d{0.5f, 0.0f}, frac2d{1.0f, 0.5f}));
	renderer->UI->children[0]->children.emplace_back(std::make_unique<Rectangle>("Test Rectangle", frac2d{0.0f, 0.0f}, frac2d{0.0f, 0.0f}, frac2d{1.0f, 1.0f}, 0xFFFF00FF));
	// renderer->UI->children.emplace_back(std::move(interface));
}
