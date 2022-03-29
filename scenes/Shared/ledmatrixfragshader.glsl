precision mediump float;

varying vec2 vTexCoord;
uniform sampler2D uTexture;
uniform vec4 uColor;
uniform vec2 uTextureSize;

void main(void)
{
  vec2 pxLoc = (vTexCoord.xy * uTextureSize) - floor(vTexCoord.xy * uTextureSize);
  float gate = pxLoc.x >= 0.25 && pxLoc.x <= 0.75 && pxLoc.y >= 0.25 && pxLoc.y <= 0.75 ? 1.0 : 0.0;
  float glow = (1.0 - length(pxLoc - vec2(0.5,0.5)) / 0.70) * 1.0;
  vec4 color = texture2D(uTexture, vTexCoord) * uColor;
  gl_FragColor =  color * vec4(gate, gate, gate, 1) + color * vec4(glow, glow, glow, 1);
}
