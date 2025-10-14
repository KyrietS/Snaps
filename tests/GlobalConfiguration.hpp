#pragma once

struct GlobalConfiguration {
    bool PreviewOnFailure = false;
    bool PreviewAlways = false;

    static const GlobalConfiguration& Get() {
        return GetRef();
    }

    static void Set(GlobalConfiguration config) {
        GetRef() = std::move(config);
    }

private:
    static GlobalConfiguration& GetRef() {
        static GlobalConfiguration instance{};
        return instance;
    }
};