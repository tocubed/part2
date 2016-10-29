#include <script/stdlib.hpp>

#include <chaiscript/chaiscript_stdlib.hpp>

std::shared_ptr<chaiscript::Module> Script::createChaiScriptStdLib()
{
	return chaiscript::Std_Lib::library();
}
