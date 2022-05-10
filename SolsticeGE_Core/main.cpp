/*
Solstice Game Engine
Created by Nathan Laha
*/

#include "EngineWrapper.h"

/// <summary>
/// Program entry
/// </summary>
/// <param name=""></param>
/// <returns></returns>
int main(void)
{

    SolsticeGE::EngineWrapper app;

    if (!app.init()) {
        spdlog::error("Could not initialize engine!");
        return -1;
    }

    if (!app.run()) {
        spdlog::error("Could not start engine!");
        return -1;
    }


    return 0;
}