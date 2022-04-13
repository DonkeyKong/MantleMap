// This is the "standard" shader, suitable for most 2D graphics

precision mediump float;

// Mandatory inputs
//attribute vec4 aPosition;
uniform vec4 uTint; // Multiplies by the final color (applied in frag shader)
uniform mat4 uPixelFromModelTransform; // Moves, scales, and rotates the mesh being drawn
uniform mat4 uCameraFromPixelTransform; // Captures the display output size

// Optional: Texture Mapping
#ifdef FEATURE_TEXTURE
uniform sampler2D uTexture;
uniform vec2 uTextureSize;
//attribute vec2 aTexCoord;
varying vec2 vTexCoord;
#endif

// Optional: Vertex Colors
#ifdef FEATURE_VERTEX_COLOR
//attribute vec4 aColor;
varying vec4 vColor;
#endif

void main(void)
{
    vec4 color = uTint;

    #ifdef FEATURE_TEXTURE
    color *= texture2D(uTexture, vTexCoord);
    #endif

    #ifdef FEATURE_VERTEX_COLOR
    color *= vColor;
    #endif

    gl_FragColor =  color;
}
