// This is the "standard" shader, suitable for most 2D graphics

// Mandatory inputs
attribute vec4 aPosition;
uniform vec4 uTint; // Multiplies by the final color (applied in frag shader)
uniform mat4 uPixelFromModelTransform; // Moves, scales, and rotates the mesh being drawn
uniform mat4 uCameraFromPixelTransform; // Captures the display output size

// Optional: Texture Mapping
#ifdef FEATURE_TEXTURE
uniform sampler2D uTexture;
uniform vec2 uTextureSize;
attribute vec2 aTexCoord;
varying vec2 vTexCoord;
#endif

// Optional: Vertex Colors
#ifdef FEATURE_VERTEX_COLOR
attribute vec4 aColor;
varying vec4 vColor;
#endif

void main(void)
{
    // Perform the matrix multiplication, but pass through color and textrue coords
#ifdef FEATURE_PIXEL_SNAP
    gl_Position = uCameraFromPixelTransform * floor(uPixelFromModelTransform * aPosition);
#else
    gl_Position = uCameraFromPixelTransform * uPixelFromModelTransform * aPosition;
#endif

#ifdef FEATURE_TEXTURE
    vTexCoord = aTexCoord;
#endif

#ifdef FEATURE_VERTEX_COLOR
    vColor = aColor;
#endif
}
