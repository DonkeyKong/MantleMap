// Mandatory inputs
attribute vec4 aPosition;
uniform vec4 uTint; // Multiplies by the final color (applied in frag shader)
uniform mat4 uPixelFromModelTransform; // Moves, scales, and rotates the mesh being drawn
uniform mat4 uCameraFromPixelTransform; // Captures the display output size

// Extra inputs
attribute float aPointSize;
attribute vec4 aColor;
varying vec4 vColor;


void main(void)
{
  vColor = aColor;
  gl_PointSize = ceil(aPointSize);
  gl_Position = uCameraFromPixelTransform * floor(uPixelFromModelTransform * aPosition);
}