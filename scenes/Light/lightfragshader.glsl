#define M_PI 3.1415926535897932384626433832795
#define M_HALFPI 1.57079633
#define M_DAWNANGLE 0.2

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

// Extra Uniforms
uniform float uSunPropigationRad;
uniform bool uDrawSun;
uniform bool uDrawMoon;
uniform vec2 uSunLonLat;
uniform vec2 uMoonLonLat;
uniform float uLightBoost;
uniform sampler2D uTexture1;
uniform sampler2D uTexture2;

void main(void)
{
    // Retrieve longitude and lattitude
    vec4 lonLatVec = texture2D(uTexture2, vTexCoord);
    lonLatVec.xyz = (lonLatVec.xyz - 0.5) * 2.0;
    
    vec3 sunVec = vec3(cos(uSunLonLat.x)*cos(uSunLonLat.y),
                       sin(uSunLonLat.x)*cos(uSunLonLat.y),
                       sin(uSunLonLat.y));
                       
    vec3 moonVec = vec3(cos(uMoonLonLat.x)*cos(uMoonLonLat.y),
                       sin(uMoonLonLat.x)*cos(uMoonLonLat.y),
                       sin(uMoonLonLat.y));
                       
    float sunAngle = acos(dot(lonLatVec.xyz,normalize(sunVec)));
    float moonAngle = acos(dot(lonLatVec.xyz,normalize(moonVec)));
    
    if (lonLatVec.a == 0.0)
    {
      gl_FragColor = vec4(0,0,0,1);
    }
    else if (uDrawSun && abs(sunAngle) < 0.14)
    {
      gl_FragColor = vec4(1,1,0,1);
    }
    else if (uDrawMoon && abs(moonAngle) < 0.1)
    {
      gl_FragColor = vec4(1,1,1,1);
    }
    else
    {
      if (sunAngle < uSunPropigationRad)
        gl_FragColor = mix(texture2D(uTexture, vTexCoord), vec4(1.0, 1.0, 1.0, 1.0), uLightBoost);
      else if (sunAngle < (uSunPropigationRad + M_DAWNANGLE))
        gl_FragColor = mix(texture2D(uTexture, vTexCoord), vec4(1.0, 1.0, 1.0, 1.0), uLightBoost) * mix(vec4(1.0, 1.0, 1.0, 1.0), vec4(0.75, 0.5, 0, 1.0), (sunAngle - uSunPropigationRad)/(M_DAWNANGLE + 0.05));
      else
        gl_FragColor = texture2D(uTexture1, vTexCoord);
     }
}
