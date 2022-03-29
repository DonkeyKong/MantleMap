attribute vec4 aVertex;
attribute vec2 aTexCoord;
varying vec2 vTexCoord;
uniform vec2 uLocation;
uniform mat4 uCameraFromPixelTransform;

void main(void)
{
  vTexCoord = aTexCoord;
  gl_Position = uCameraFromPixelTransform * (aVertex + vec4(uLocation.x, uLocation.y, 0, 0));
}
