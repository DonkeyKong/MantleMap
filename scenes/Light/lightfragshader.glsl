#define M_PI 3.1415926535897932384626433832795
#define M_HALFPI 1.57079633
#define M_DAWNANGLE 0.2

precision mediump float;
varying vec4 v_Colour;

// Uniforms
uniform vec2 uScale;
uniform float uTime;
uniform float uSunPropigationRad;
uniform bool uDrawSun;
uniform bool uDrawMoon;
uniform vec2 uSunLonLat;
uniform vec2 uMoonLonLat;
uniform float uLightBoost;
uniform sampler2D uTexture1;
uniform sampler2D uTexture2;
uniform sampler2D uLonLatLut;

vec3 hsv2rgb(vec3 c)
{
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main(void)
{
    vec2 position = (gl_FragCoord.xy - vec2(-0.5, 0.5))/(uScale - vec2(1,1));
    vec2 texCoord = vec2(position.x, 1.0 - position.y);

    // Retrieve longitude and lattitude
    vec4 lonLatVec = texture2D(uLonLatLut, texCoord);
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
        gl_FragColor = mix(texture2D(uTexture1, texCoord), vec4(1.0, 1.0, 1.0, 1.0), uLightBoost);
      else if (sunAngle < (uSunPropigationRad + M_DAWNANGLE))
        gl_FragColor = mix(texture2D(uTexture1, texCoord), vec4(1.0, 1.0, 1.0, 1.0), uLightBoost) * mix(vec4(1.0, 1.0, 1.0, 1.0), vec4(0.75, 0.5, 0, 1.0), (sunAngle - uSunPropigationRad)/(M_DAWNANGLE + 0.05));
      else
        gl_FragColor = texture2D(uTexture2, texCoord);
     }
}
