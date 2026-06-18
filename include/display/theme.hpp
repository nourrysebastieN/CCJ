#pragma once

#include <string>

struct Theme {
    std::string name;  // e.g. "default", "carbon"
    std::string dir;   // resolved path: "assets/themes/default"

    // Build a Theme from a base directory and a theme name.
    // Assets are expected at {base}/{name}/*.png
    static Theme from(const std::string& base, const std::string& name) {
        return { name, base + "/" + name };
    }
};
