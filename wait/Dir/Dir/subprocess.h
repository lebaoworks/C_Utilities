#pragma once

#include <Windows.h>
#include <vector>
#include <string>

void RunCmdAndGetText(std::wstring cmd_line, std::vector<std::wstring>& output);