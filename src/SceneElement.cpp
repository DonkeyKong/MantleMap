#include "SceneElement.hpp"

SceneElement::SceneElement(MapState& map) : map(map)
{
    // Nothing to do
}

SceneElement::~SceneElement()
{
    // Nothing to do
}

void SceneElement::Draw()
{
    initGL();
    drawInternal();
}
