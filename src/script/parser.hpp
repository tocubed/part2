#pragma once

#include <memory>

namespace chaiscript {
	namespace parser {
		class ChaiScript_Parser_Base;
	}
}

namespace Script {
	std::unique_ptr<chaiscript::parser::ChaiScript_Parser_Base>
	createChaiScriptParser();
}
