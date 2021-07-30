varying vec2 vTexCoord;
uniform sampler2D uFontTexture;
uniform vec4 uColor;

void main(void)
{
  gl_FragColor = texture2D(uFontTexture, vTexCoord) * uColor;
}
