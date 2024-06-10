#pragma once
#include <optional>
#include <filesystem>
#include <memory>
#include<string>

namespace fs = std::filesystem;

int run_model(const fs::path config_file_path, std::optional<std::string> agent_file,std::optional<std::string> network_file ,const fs::path output_dir_path);