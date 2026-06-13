#pragma once

#include <functional>
#include <string>
#include <vector>

// prefix     — partial word being typed
// linePrefix — everything before that word (used to detect first vs. later token)
using Completer = std::function<std::vector<std::string>(const std::string& prefix, const std::string& linePrefix)>;

std::string ReadLine(const std::string& prompt,
                     const Completer& completer = {},
                     std::vector<std::string>* history = nullptr);
