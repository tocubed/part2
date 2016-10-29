#pragma once

#include <memory>

namespace chaiscript {
	class Module;
}

namespace Script {
	std::shared_ptr<chaiscript::Module> createChaiScriptStdLib();
}
