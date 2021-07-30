attribute vec4 aVertex;
attribute vec4 aColor;
varying vec4 vColor;
uniform vec2 uLocation;
uniform mat4 uCameraFromPixelTransform;

void main(void)
{
  vColor = aColor;
  gl_Position = uCameraFromPixelTransform * (aVertex + vec4(uLocation.x, uLocation.y, 0, 0));
}
