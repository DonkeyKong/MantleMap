attribute vec4 aVertex;
attribute vec2 aTexCoord;
varying vec2 vTexCoord;
uniform vec2 uLocation;
uniform vec2 uFontSizePx;
uniform mat4 uCameraFromPixelTransform;

void main(void)
{
  vTexCoord = aTexCoord;
  gl_Position = uCameraFromPixelTransform * (aVertex * vec4(uFontSizePx.x, uFontSizePx.y, 1,1) + vec4(floor(uLocation.x), floor(uLocation.y), 0, 0));
}
