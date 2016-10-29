#include <script/parser.hpp>

#include <chaiscript/language/chaiscript_parser.hpp>

#include <memory>

std::unique_ptr<chaiscript::parser::ChaiScript_Parser_Base>
Script::createChaiScriptParser() 
{
	return std::make_unique<chaiscript::parser::ChaiScript_Parser<
	    chaiscript::eval::Noop_Tracer,
	    chaiscript::optimizer::Optimizer_Default>>();
}
