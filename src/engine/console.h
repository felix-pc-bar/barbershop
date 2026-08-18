#include <deque>
#include <string>
#include <chrono>

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

	void log(std::string text);
	void info(std::string text);
	void warn(std::string text);
	void error(std::string text);
	void prompt(std::string text);

private:
	std::deque<Line> lines;
};
