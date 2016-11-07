#pragma once

#include <core/manager.hpp>
#include <core/system.hpp>
#include <script/chaiscript.hpp>

#include <functional>

class ScriptSystem : System
{
public:
	ScriptSystem(Manager& manager);

	void update(sf::Time delta);

	void dialogBox(std::string text, std::function<void ()> continuation);
	void menuBox(
	    const std::vector<std::string>& options,
	    const std::vector<std::function<void()>>& continuations);
	void promptBox(
		const std::string& text,
	    const std::vector<std::string>& options,
	    const std::vector<std::function<void()>>& continuations);

	void runScript(std::string src);

private:
	chaiscript::ChaiScript_Basic& chai;	

	std::function<void()> dialogContinuation;
	std::vector<std::function<void()>> menuContinuations;

	bool prompt, promptUp;
	std::vector<std::string> promptOptions;
	std::vector<std::function<void()>> promptContinuations;
};
