#pragma once

#include "Shader.h"
#include "Material.h"

namespace nle::globals
{
    extern Shader*      DEFAULT_SHADER;
    extern Material*    DEFAULT_MATERIAL;

    void init();
    void cleanup();
} // namespace nle
