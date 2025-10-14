#define _CRT_SECURE_NO_WARNINGS // NOLINT(*-reserved-identifier) Silly Windows has problems with std::getenv
#include <iostream>
#include <gtest/gtest.h>
#include <cstdlib>

#include "GlobalConfiguration.hpp"

namespace {

std::optional<std::string> GetEnvironmentVar(const std::string_view varName) {
    const char* value = std::getenv(varName.data());
    return value ? std::make_optional(std::string(value)) : std::nullopt;
}

bool IsEnvFlagEnabled(const std::string_view varName) {
    return GetEnvironmentVar(varName).and_then([](const std::string& value) {
        return std::make_optional(value != "0");
    }).value_or(false);
}

std::optional<bool> GetArgFlag(const std::string_view flagName, const int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        const std::string_view arg(argv[i]);
        const std::string disabledFlag = std::string(flagName) + "=0";
        const bool isFlagDisabled = arg == disabledFlag;
        const bool isFlagPresent = arg.starts_with(flagName);
        if (isFlagPresent) {
            return not isFlagDisabled;
        }
    }
    return std::nullopt;
}

void PrintHelp() {
    std::cout << "Scene Preview:" << std::endl;
    std::cout << "  --preview_on_failure" << std::endl;
    std::cout << "      Show a preview window if a scene test fails. Can also be set with" << std::endl;
    std::cout << "      PREVIEW_ON_FAILURE environment variable." << std::endl;
    std::cout << "  --preview_always" << std::endl;
    std::cout << "      Always show a preview window for each scene test. Can also be set" << std::endl;
    std::cout << "      with PREVIEW_ALWAYS environment variable." << std::endl << std::endl;
}

void ParseArgs(const int argc, char** argv) {
    const bool previewOnFailure = GetArgFlag("--preview_on_failure", argc, argv).value_or(IsEnvFlagEnabled("PREVIEW_ON_FAILURE"));
    const bool previewAlways = GetArgFlag("--preview_always", argc, argv).value_or(IsEnvFlagEnabled("PREVIEW_ALWAYS"));

    GlobalConfiguration::Set({
        .PreviewOnFailure = previewOnFailure,
        .PreviewAlways = previewAlways
    });

    if (GetArgFlag("--help", argc, argv).has_value()) {
        PrintHelp();
    }
}
}

int main(int argc, char** argv) {
    ParseArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
