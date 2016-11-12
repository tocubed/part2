#include <script/bindings.hpp>
#include <script/chaiscript.hpp>
#include <script/parser.hpp>
#include <script/stdlib.hpp>

#include <chaiscript/chaiscript.hpp>

#include <memory>

namespace Script {

static std::unique_ptr<chaiscript::ChaiScript_Basic> createChaiScript()
{
	auto chai = std::unique_ptr<chaiscript::ChaiScript_Basic>(
	    new chaiscript::ChaiScript_Basic(
	        createChaiScriptStdLib(), createChaiScriptParser(), {},
	        {"assets/scripts/"}));

	chai->add(createChaiScriptBindings());

	return chai;
}

}

chaiscript::ChaiScript_Basic& Script::getChaiScript()
{
	static auto chai = createChaiScript();

	return *chai;
}
