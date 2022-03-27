precision mediump float;

// Uniforms
uniform vec2 uScale;
uniform sampler2D uLonLatLut;
uniform mat4 uCameraFromPixelTransform;

void main(void)
{
    vec2 position = (gl_FragCoord.xy - vec2(-0.5, 0.5))/(uScale - vec2(1,1));
    vec2 texCoord = vec2(position.x, 1.0 - position.y);

    // Retrieve longitude and lattitude
    vec4 lonLatVec = texture2D(uLonLatLut, texCoord);
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
