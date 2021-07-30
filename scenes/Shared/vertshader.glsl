attribute vec4 vertex;
uniform mat4 uCameraFromPixelTransform;

void main(void)
{
        gl_Position = uCameraFromPixelTransform * vertex;
}

