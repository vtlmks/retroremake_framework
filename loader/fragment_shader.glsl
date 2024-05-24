// Specify default precision for fragment shaders
precision mediump float;

out vec4 outcolor;
in vec2 frag_texture_coord;

uniform vec2 resolution;
uniform vec2 src_image_size;
uniform float brightness;
uniform vec4 tone_data;
uniform bool crt_emulation;
uniform sampler2D iChannel0;

mediump vec3 CrtsFetch(mediump vec2 uv) {
	const float bias = 0.003333333;
	return max(texture(iChannel0, uv).rgb, vec3(bias));
}

#define CrtsRcpF1(x) (1.0 / (x))
#define CrtsSatF1(x) clamp((x), 0.0, 1.0)
const float PI2 = 6.28318530717958;
const float HALF = 0.5;

mediump float CrtsMax3F1(mediump float a, mediump float b, mediump float c) {
	return max(a, max(b, c));
}

mediump vec3 CrtsMask(mediump vec2 pos, mediump float dark) {
	#ifdef CRTS_MASK_GRILLE
		mediump vec3 m = vec3(dark);
		mediump float x = fract(pos.x * (1.0 / 3.0));
		m.r = (x < (1.0 / 3.0)) ? 1.0 : dark;
		m.g = (x >= (1.0 / 3.0) && x < (2.0 / 3.0)) ? 1.0 : dark;
		m.b = (x >= (2.0 / 3.0)) ? 1.0 : dark;
		return m;
	#endif

	#ifdef CRTS_MASK_GRILLE_LITE
		mediump vec3 m = vec3(1.0);
		mediump float x = fract(pos.x * (1.0 / 3.0));
		m.r = (x < (1.0 / 3.0)) ? dark : 1.0;
		m.g = (x >= (1.0 / 3.0) && x < (2.0 / 3.0)) ? dark : 1.0;
		m.b = (x >= (2.0 / 3.0)) ? dark : 1.0;
		return m;
	#endif

	#ifdef CRTS_MASK_NONE
		return vec3(1.0);
	#endif

	#ifdef CRTS_MASK_SHADOW
		pos.x += pos.y * 3.0;
		mediump vec3 m = vec3(dark);
		mediump float x = fract(pos.x * (1.0 / 6.0));
		m.r = (x < (1.0 / 3.0)) ? 1.0 : dark;
		m.g = (x >= (1.0 / 3.0) && x < (2.0 / 3.0)) ? 1.0 : dark;
		m.b = (x >= (2.0 / 3.0)) ? 1.0 : dark;
		return m;
	#endif
}

mediump vec3 CrtsFilter(mediump vec2 ipos, mediump vec2 inputSizeDivOutputSize, mediump vec2 halfInputSize, mediump vec2 rcpInputSize, mediump vec2 rcpOutputSize, mediump vec2 twoDivOutputSize, mediump float inputHeight, mediump vec2 warp, mediump float thin, mediump float blur, mediump float mask, mediump vec4 tone) {
	mediump vec2 pos = ipos * twoDivOutputSize - vec2(1.0);
	pos *= vec2(1.0 + (pos.y * pos.y) * warp.x, 1.0 + (pos.x * pos.x) * warp.y);
	mediump float vin = 1.0 - ((1.0 - CrtsSatF1(pos.x * pos.x)) * (1.0 - CrtsSatF1(pos.y * pos.y)));
	vin = CrtsSatF1((-vin) * inputHeight + inputHeight);
	pos = pos * halfInputSize + halfInputSize;

	mediump float y0 = floor(pos.y - 0.5) + 0.5;
	mediump float x0 = floor(pos.x - 1.5) + 0.5;
	mediump vec2 p = vec2(x0 * rcpInputSize.x, y0 * rcpInputSize.y);

	mediump vec3 colA[4], colB[4];
	for (int i = 0; i < 4; i++) {
		colA[i] = CrtsFetch(p);
		p.x += rcpInputSize.x;
	}
	p.y += rcpInputSize.y;
	for (int i = 3; i >= 0; i--) {
		p.x -= rcpInputSize.x;
		colB[i] = CrtsFetch(p);
	}

	mediump float off = pos.y - y0;
	mediump float scanA = cos(min(HALF, off * thin) * PI2) * HALF + HALF;
	mediump float scanB = cos(min(HALF, (-off) * thin + thin) * PI2) * HALF + HALF;

	mediump float off0 = pos.x - x0;
	mediump float pix[4];
	for (int i = 0; i < 4; i++) {
		mediump float diff = off0 - float(i);
		pix[i] = exp2(blur * diff * diff);
	}
	mediump float pixT = CrtsRcpF1(pix[0] + pix[1] + pix[2] + pix[3]);

	#ifdef CRTS_WARP
		pixT *= vin;
	#endif

	scanA *= pixT;
	scanB *= pixT;

	mediump vec3 color = (colA[0] * pix[0] + colA[1] * pix[1] + colA[2] * pix[2] + colA[3] * pix[3]) * scanA + (colB[0] * pix[0] + colB[1] * pix[1] + colB[2] * pix[2] + colB[3] * pix[3]) * scanB;
	color *= CrtsMask(ipos, mask);

	#ifdef CRTS_TONE
		mediump float peak = max(1.0 / (256.0 * 65536.0), CrtsMax3F1(color.r, color.g, color.b));
		mediump vec3 ratio = color * CrtsRcpF1(peak);
		#ifdef CRTS_CONTRAST
			peak = pow(peak, tone.x);
		#endif
		peak = peak * CrtsRcpF1(peak * tone.y + tone.z);
		#ifdef CRTS_SATURATION
			ratio = pow(ratio, vec3(tone.w));
		#endif
		return ratio * peak;
	#else
		return color;
	#endif
}

mediump vec3 linearToSRGB(mediump vec3 color) {
	return pow(color, vec3(1.0 / 2.2));
}

void main() {
	mediump vec2 fragCoord = vec2(frag_texture_coord.x, 1.0 - frag_texture_coord.y);
	if (crt_emulation) {
		outcolor.rgb = CrtsFilter(
			fragCoord.xy * resolution,
			src_image_size / resolution,
			src_image_size * vec2(0.5),
			1.0 / src_image_size,
			1.0 / resolution,
			2.0 / resolution,
			src_image_size.y,
			vec2(1.0 / 24.0, 1.0 / 16.0),	// warp value
			INPUT_THIN,
			INPUT_BLUR,
			INPUT_MASK,
			tone_data
		);

		outcolor.rgb *= brightness;
		outcolor = vec4(outcolor.rgb, 1.0); // Keep original color with alpha set to 1.0

	} else {
		outcolor = texture(iChannel0, fragCoord);
	}
}

