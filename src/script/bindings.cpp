#include <script/bindings.hpp>

#include <chaiscript/chaiscript.hpp>

#include <memory>

std::shared_ptr<chaiscript::Module> Script::createChaiScriptBindings()
{
	auto module = std::make_shared<chaiscript::Module>();

	return module;
}
