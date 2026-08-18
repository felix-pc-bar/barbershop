#pragma once

#include <functional>
#include <string>
#include <deque>

#include "render/components/bmpfont.h"
#include "render/render.h"
#include "ui.h"

class Console
{
public:
	enum class lineType
	{
		Log,
		Info,
		Warning,
		Error,
		Prompt
	};

	class Line
	{
		Line() = default;
		std::string text;
		lineType type;
		std::chrono::system_clock::time_point when;
	};

	int maxLines;

	Console(
		cRenderer* renderer,
		bmpFont* font = nullptr
	);

	void log(std::string text);
	void info(std::string text);
	void warn(std::string text);
	void error(std::string text);
	void prompt(std::string text);

private:
	std::deque<Line> lines;
};

class DialogueBox
{
public:
	DialogueBox(
		cRenderer* renderer,
		std::string title,
		std::string body,
		std::string t1,
		std::function<void()> lbd1,
		std::string t2 = "",
		std::function<void()> lbd2 = nullptr,
		std::string t3 = "",
		std::function<void()> lbd3 = nullptr
	);
};
