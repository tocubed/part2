#include <script/bindings.hpp>
#include <core/manager.hpp>

#include <chaiscript/chaiscript.hpp>

#include <memory>

std::shared_ptr<chaiscript::Module> Script::createChaiScriptBindings()
{
	auto module = std::make_shared<chaiscript::Module>();

	module->add(chaiscript::vector_conversion<std::vector<std::string>>());
	module->add(
	    chaiscript::vector_conversion<std::vector<std::function<void()>>>());

	chaiscript::utility::add_class<Direction>(*module, "Direction", {
		{UP, "UP"},
		{DOWN, "DOWN"},
		{RIGHT, "RIGHT"},
		{LEFT, "LEFT"},
		{NONE, "NONE"}
	});

	return module;
}
