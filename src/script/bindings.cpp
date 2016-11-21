#include <script/bindings.hpp>
#include <core/ecs/typedefs.hpp>

#include <chaiscript/chaiscript.hpp>

#include <memory>

std::shared_ptr<chaiscript::Module> Script::createChaiScriptBindings()
{
	auto module = std::make_shared<chaiscript::Module>();

	module->add(chaiscript::vector_conversion<std::vector<std::string>>());
	module->add(
	    chaiscript::vector_conversion<std::vector<std::function<void()>>>());

	return module;
}
