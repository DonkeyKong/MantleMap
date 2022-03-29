precision mediump float;

varying vec2 vTexCoord;
uniform sampler2D uTexture;
uniform vec4 uColor;
uniform vec2 uTextureSize;

void main(void)
{
  vec4 color = texture2D(uTexture, vTexCoord) * uColor;
  gl_FragColor =  color;
}
