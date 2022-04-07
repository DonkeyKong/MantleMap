attribute vec4 aVertex;
attribute vec4 aColor;
attribute float aPointSize;

varying vec4 vColor;

uniform mat4 uCameraFromPixelTransform;

void main(void)
{
  vColor = aColor;
  gl_PointSize = ceil(aPointSize);
  gl_Position = uCameraFromPixelTransform * floor(aVertex);
}
