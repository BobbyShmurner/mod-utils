#pragma once

#include <string>

namespace ModloaderUtils {
	struct Dependency {
		std::string id;
		std::string version;
		std::string downloadIfMissing;
	};
}