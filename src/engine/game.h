#pragma once
#include "SDL_keycode.h"
#include "render/components/bmpfont.h"
#include "stubble/stubble.h"
#include "render/render.h"

#include <chrono>
#include <SDL_events.h>

class Game
{
public:
	Game();
	~Game();
	void run();
	void quit();
	cRenderer* renderer;
	StubbleParser* stubbleparser;
private:

	const Uint8* gk; // Used to read off inputs
	SDL_Event event; // SDL event buffer
	SDL_Keymod mods;

	// ====
	// TIME
	// ====

	int frame;
	std::chrono::steady_clock::time_point lastTime;
	int fpsLimit;
	float fpsLimTick;
	float gameTime; //Time since game start, use for framerate independent motion eg trig anim
	float dtFac; // dt as ratio; shouldn't change anything if you multiply with it and you're running at 60fps

	float fpsRunningTotal;
	int runningAvgPeriodFrames;

	// buff stuff
	std::vector<pixelBuffer*> pxbufs;

	std::vector<int> rulers; // These are horizontal lines to help with typography; the int stores the number of pixels above the bottom of the normal font the line should be drawn (respecting zoom)
	int previewRuleHeight;
	bool previewingRule; // For the rule being previewed before it is placed

	bmpFont* workingFont;
	bmpFont* stopgapFont; // for UI temporarily
	int defaultWidth;
	int defaultHeight;
	// these are the "camera" offset
	int dx;
	int dy;

	// zoom
	int scale;

	// in screen space,y-up
	int mousex;
	int mousey;

	// 0,0 when origin is at centre of screen; origin @ bottom left of screen -> (-ve,-ve)
	int originDX;
	int originDY;

	std::vector<Point2d> bufrels;
	bool lmbdown;
	bool mmbdown;
	bool rmbdown;

	std::string testString;

	enum class state
	{
		roughing_normal,
		tweaking_normal
	};

	enum class tool
	{
		pen,
		line,
		circle,
		placing_ruler
	};

	state currentState;
	tool currentTool;
	bool unsavedWork;
	const char* asciiDescriptions[128] = {
		"Null character- used as placeholder",                                      // 0
		"Start of heading",                                    // 1
		"Start of text",                                      // 2
		"End of text",                                        // 3
		"End of transmission",                                // 4
		"Enquiry",                                             // 5
		"Acknowledge",                                         // 6
		"Bell",                                                // 7
		"Backspace",                                           // 8
		"Horizontal tab",                                     // 9
		"Line feed",                                           // 10
		"Vertical tab",                                       // 11
		"Form feed",                                          // 12
		"Carriage return",                                    // 13
		"Shift out",                                          // 14
		"Shift in",                                           // 15
		"Data link escape",                                   // 16
		"Device control one",                                 // 17
		"Device control two",                                 // 18
		"Device control three",                               // 19
		"Device control four",                                // 20
		"Negative acknowledge",                               // 21
		"Synchronous idle",                                   // 22
		"End of transmission block",                          // 23
		"Cancel",                                              // 24
		"End of medium",                                      // 25
		"Substitute",                                         // 26
		"Escape",                                             // 27
		"File separator",                                     // 28
		"Group separator",                                    // 29
		"Record separator",                                   // 30
		"Unit separator",                                     // 31
		"Space",                                              // 32
		"Exclamation mark",                                   // 33
		"Double quote",                                       // 34
		"Number sign",                                        // 35
		"Dollar sign",                                        // 36
		"Percent sign",                                       // 37
		"Ampersand",                                          // 38
		"Apostrophe",                                         // 39
		"Left parenthesis",                                   // 40
		"Right parenthesis",                                  // 41
		"Asterisk",                                           // 42
		"Plus sign",                                          // 43
		"Comma",                                              // 44
		"Hyphen-minus",                                       // 45
		"Full stop",                                          // 46
		"Solidus",                                            // 47
		"0",                                                   // 48
		"1",                                                   // 49
		"2",                                                   // 50
		"3",                                                   // 51
		"4",                                                   // 52
		"5",                                                   // 53
		"6",                                                   // 54
		"7",                                                   // 55
		"8",                                                   // 56
		"9",                                                   // 57
		"Colon",                                              // 58
		"Semicolon",                                          // 59
		"Less-than sign",                                     // 60
		"Equals sign",                                        // 61
		"Greater-than sign",                                  // 62
		"Question mark",                                      // 63
		"At sign",                                            // 64
		"A",                                                   // 65
		"B",                                                   // 66
		"C",                                                   // 67
		"D",                                                   // 68
		"E",                                                   // 69
		"F",                                                   // 70
		"G",                                                   // 71
		"H",                                                   // 72
		"I",                                                   // 73
		"J",                                                   // 74
		"K",                                                   // 75
		"L",                                                   // 76
		"M",                                                   // 77
		"N",                                                   // 78
		"O",                                                   // 79
		"P",                                                   // 80
		"Q",                                                   // 81
		"R",                                                   // 82
		"S",                                                   // 83
		"T",                                                   // 84
		"U",                                                   // 85
		"V",                                                   // 86
		"W",                                                   // 87
		"X",                                                   // 88
		"Y",                                                   // 89
		"Z",                                                   // 90
		"Left square bracket",                                 // 91
		"Backslash",                                           // 92
		"Right square bracket",                                // 93
		"Circumflex accent",                                   // 94
		"Underscore",                                          // 95
		"Grave accent",                                        // 96
		"a",                                                   // 97
		"b",                                                   // 98
		"c",                                                   // 99
		"d",                                                   // 100
		"e",                                                   // 101
		"f",                                                   // 102
		"g",                                                   // 103
		"h",                                                   // 104
		"i",                                                   // 105
		"j",                                                   // 106
		"k",                                                   // 107
		"l",                                                   // 108
		"m",                                                   // 109
		"n",                                                   // 110
		"o",                                                   // 111
		"p",                                                   // 112
		"q",                                                   // 113
		"r",                                                   // 114
		"s",                                                   // 115
		"t",                                                   // 116
		"u",                                                   // 117
		"v",                                                   // 118
		"w",                                                   // 119
		"x",                                                   // 120
		"y",                                                   // 121
		"z",                                                   // 122
		"Left curly bracket",                                  // 123
		"Vertical bar",                                        // 124
		"Right curly bracket",                                 // 125
		"Tilde",                                               // 126
		"Delete"                                               // 127
	};
	// == Functions ==
	void dealFontBuffers(bmpFont* font); // Copies the buffers of a font into the pxbufs vector to be displayed, for inspection and editing
	void createUndoState();
};
