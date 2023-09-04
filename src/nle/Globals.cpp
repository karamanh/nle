#include "Globals.h"

namespace nle::globals
{
    Shader*     DEFAULT_SHADER;
    Material*   DEFAULT_MATERIAL;

    void init()
    {
        // initialize defaults
        DEFAULT_SHADER = new Shader("shader/default_vert.glsl", "shader/default_frag.glsl", true);
        DEFAULT_MATERIAL = new Material();
    }

    void cleanup()
    {
        delete DEFAULT_MATERIAL;
        delete DEFAULT_SHADER;
    }

} // namespace nle
