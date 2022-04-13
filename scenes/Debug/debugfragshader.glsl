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

// vec3 hsv2rgb(vec3 c)
// {
//   vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
//   vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
//   return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
// }

void main(void)
{
    // Retrieve longitude and lattitude
    vec4 lonLatVec = texture2D(uTexture, vTexCoord);
    lonLatVec.xyz = (lonLatVec.xyz - 0.5) * 2.0;
    
    if (lonLatVec.a == 0.0)
    {
      gl_FragColor = vec4(0.1,0.1,0.1,1);
    }
    else
    {
      gl_FragColor = lonLatVec;
    }
}
