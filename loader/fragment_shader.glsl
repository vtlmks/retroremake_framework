
out vec4 outcolor;
in vec2 frag_texture_coord;

uniform vec2 resolution;
uniform vec2 src_image_size;
uniform float brightness;
uniform vec4 tone_data;
uniform bool crt_emulation;

uniform sampler2D iChannel0;

// NOTE: this shader was taken from https://www.shadertoy.com/view/MtSfRK it's in the public domain.
//       modified and butchered; please don't use in other projects, I've probably destroyed it..
//--------------------------------------------------------------
// Setup the function which returns input image color
vec3 CrtsFetch(vec2 uv) {
	return texture(iChannel0, uv, -16.0).rgb;
}

////////////////////////////////////////////////////////////////
#define CrtsRcpF1(x) (1.0 / (x))
#define CrtsSatF1(x) clamp((x), 0.0, 1.0)
//--------------------------------------------------------------
float CrtsMax3F1(float a,float b,float c) {
	return max(a, max(b, c));
}

//==============================================================
//                            MASK
//--------------------------------------------------------------
// Letting LCD/OLED pixel elements function like CRT phosphors
// So \"phosphor\" resolution scales with display resolution
//--------------------------------------------------------------
// Not applying any warp to the mask (want high frequency)
// Real aperture grille has a mask which gets wider on ends
// Not attempting to be \"real\" but instead look the best
//--------------------------------------------------------------
// Shadow mask is stretched horizontally
//  RRGGBB
//  GBBRRG
//  RRGGBB
// This tends to look better on LCDs than vertical
// Also 2 pixel width is required to get triad centered
//--------------------------------------------------------------
// The LITE version of the Aperture Grille is brighter
// Uses {dark,1.0,1.0} for R channel
// Non LITE version uses {1.0,dark,dark}
//--------------------------------------------------------------
// 'pos' - This is 'fragCoord.xy'
//         Pixel {0,0} should be {0.5,0.5}
//         Pixel {1,1} should be {1.5,1.5}
//--------------------------------------------------------------
// 'dark' - Exposure of of masked channel
//          0.0=fully off, 1.0=no effect
//==============================================================
vec3 CrtsMask(vec2 pos, float dark) {
	#ifdef CRTS_MASK_GRILLE
		vec3 m = vec3(dark, dark, dark);
		float x = fract(pos.x * (1.0 / 3.0));
		if(x < (1.0 / 3.0)) {
			m.r = 1.0;
		} else if(x<(2.0 / 3.0)) {
			m.g = 1.0;
		} else {
			m.b = 1.0;
		}
		return m;
	#endif
//--------------------------------------------------------------
	#ifdef CRTS_MASK_GRILLE_LITE
		vec3 m = vec3(1.0, 1.0, 1.0);
		float x = fract(pos.x * (1.0 / 3.0));
		if(x < (1.0 / 3.0)) {
			m.r = dark;
		} else if(x < (2.0 / 3.0)) {
			m.g = dark;
		} else {
			m.b = dark;
		}
		return m;
	#endif
//--------------------------------------------------------------
	#ifdef CRTS_MASK_NONE
		return vec3(1.0, 1.0, 1.0);
	#endif
//--------------------------------------------------------------
	#ifdef CRTS_MASK_SHADOW
		pos.x += pos.y * 3.0;
		vec3 m = vec3(dark, dark, dark);
		float x = fract(pos.x * (1.0 / 6.0));
		if(x < (1.0 / 3.0)) {
			m.r = 1.0;
		} else if(x < (2.0 / 3.0)) {
			m.g = 1.0;
		} else {
			m.b = 1.0;
		}
		return m;
	#endif
}

//==============================================================
//                        FILTER ENTRY
//--------------------------------------------------------------
// Input must be linear
// Output color is linear
//--------------------------------------------------------------
// Must have fetch function setup: vec3 CrtsFetch(vec2 uv)
//  - The 'uv' range is {0.0 to 1.0} for input texture
//  - Output of this must be linear color
//--------------------------------------------------------------
// SCANLINE MATH & AUTO-EXPOSURE NOTES
// ===================================
// Each output line has contribution from at most 2 scanlines
// Scanlines are shaped by a windowed cosine function
// This shape blends together well with only 2 lines of overlap
//--------------------------------------------------------------
// Base scanline intensity is as follows
// which leaves output intensity range from {0 to 1.0}
// --------
// thin := range {thick 0.5 to thin 1.0}
// off  := range {0.0 to <1.0},
//         sub-pixel offset between two scanlines
//  --------
//  a0=cos(min(0.5,     off *thin)*2pi)*0.5+0.5;
//  a1=cos(min(0.5,(1.0-off)*thin)*2pi)*0.5+0.5;
//--------------------------------------------------------------
// This leads to a image darkening factor of roughly:
//  {(1.5-thin)/1.0}
// This is further reduced by the mask:
//  {1.0/2.0+mask*1.0/2.0}
// Reciprocal of combined effect is used for auto-exposure
//  to scale up the mid-level in the tonemapper
//==============================================================
vec3 CrtsFilter(
	vec2 ipos,							// SV_POSITION, fragCoord.xy
	vec2 inputSizeDivOutputSize,	// inputSize / outputSize (in pixels)
	vec2 halfInputSize,				// 0.5 * inputSize (in pixels)
	vec2 rcpInputSize,				// 1.0 / inputSize (in pixels)
	vec2 rcpOutputSize,				// 1.0 / outputSize (in pixels)
	vec2 twoDivOutputSize,			// 2.0 / outputSize (in pixels)
	float inputHeight,				// inputSize.y
	// Warp scanlines but not phosphor mask
	//  0.0 = no warp
	//  1.0/64.0 = light warping
	//  1.0/32.0 = more warping
	// Want x and y warping to be different (based on aspect)
	vec2 warp,
	// Scanline thinness
	//  0.50 = fused scanlines
	//  0.70 = recommended default
	//  1.00 = thinner scanlines (too thin)
	// Shared with CrtsTone() function
	float thin,
	// Horizonal scan blur
	//  -3.0 = pixely
	//  -2.5 = default
	//  -2.0 = smooth
	//  -1.0 = too blurry
	float blur,
	// Shadow mask effect, ranges from,
	//  0.25 = large amount of mask (not recommended, too dark)
	//  0.50 = recommended default
	//  1.00 = no shadow mask
	// Shared with CrtsTone() function
	float mask,
	vec4 tone) {						// Tonal curve parameters generated by CrtsTone()
//--------------------------------------------------------------
	vec2 pos;																							// Optional apply warp
	pos = ipos * twoDivOutputSize - vec2(1.0,1.0);											// Convert to {-1 to 1} range

	pos *= vec2(1.0 + (pos.y * pos.y) * warp.x, 1.0+(pos.x * pos.x) * warp.y);		// Distort pushes image outside {-1 to 1} range

	// TODO: Vignette needs optimization
	float vin = 1.0 - ((1.0 - CrtsSatF1(pos.x * pos.x)) * (1.0 - CrtsSatF1(pos.y * pos.y)));
	vin = CrtsSatF1((-vin) * inputHeight + inputHeight);

	pos = pos * halfInputSize + halfInputSize;												// Leave in {0 to inputSize}

//--------------------------------------------------------------
	float y0 = floor(pos.y - 0.5) + 0.5;														// Snap to center of first scanline
	float x0 = floor(pos.x - 1.5) + 0.5;														// Snap to center of one of four pixels
	vec2 p = vec2(x0 * rcpInputSize.x, y0 * rcpInputSize.y);								// Inital UV position

	vec3 colA0 = CrtsFetch(p);																		// Fetch 4 nearest texels from 2 nearest scanlines

	p.x += rcpInputSize.x;
	vec3 colA1 = CrtsFetch(p);

	p.x += rcpInputSize.x;
	vec3 colA2 = CrtsFetch(p);

	p.x += rcpInputSize.x;
	vec3 colA3 = CrtsFetch(p);

	p.y += rcpInputSize.y;
	vec3 colB3 = CrtsFetch(p);

	p.x -= rcpInputSize.x;
	vec3 colB2 = CrtsFetch(p);

	p.x -= rcpInputSize.x;
	vec3 colB1 = CrtsFetch(p);

	p.x -= rcpInputSize.x;
	vec3 colB0 = CrtsFetch(p);

//--------------------------------------------------------------
	// Vertical filter
	// Scanline intensity is using sine wave
	// Easy filter window and integral used later in exposure
	float off = pos.y - y0;
	float pi2 = 6.28318530717958;
	float hlf = 0.5;
	float scanA = cos(min(0.5,   off  * thin       ) * pi2) * hlf + hlf;
	float scanB = cos(min(0.5, (-off) * thin + thin) * pi2) * hlf + hlf;

//--------------------------------------------------------------
	// Horizontal kernel is simple gaussian filter
	float off0 = pos.x - x0;
	float off1 = off0 - 1.0;
	float off2 = off0 - 2.0;
	float off3 = off0 - 3.0;
	float pix0 = exp2(blur * off0 * off0);
	float pix1 = exp2(blur * off1 * off1);
	float pix2 = exp2(blur * off2 * off2);
	float pix3 = exp2(blur * off3 * off3);
	float pixT = CrtsRcpF1(pix0 + pix1 + pix2 + pix3);
	#ifdef CRTS_WARP
		pixT *= vin;				// Get rid of wrong pixels on edge
	#endif
	scanA *= pixT;
	scanB *= pixT;

	// Apply horizontal and vertical filters
	vec3 color = (colA0 * pix0 + colA1 * pix1 + colA2 * pix2 + colA3 * pix3) * scanA + (colB0 * pix0 + colB1 * pix1 + colB2 * pix2 + colB3 * pix3) * scanB;

//--------------------------------------------------------------
	color *= CrtsMask(ipos, mask);																				// Apply phosphor mask

//--------------------------------------------------------------
	// Optional color processing
	#ifdef CRTS_TONE
		float peak = max(1.0 / (256.0 * 65536.0), CrtsMax3F1(color.r, color.g, color.b));		// Tonal control, start by protecting from /0
		vec3 ratio = color * CrtsRcpF1(peak);																	// Compute the ratios of {R,G,B}
		#ifdef CRTS_CONTRAST																							// Apply tonal curve to peak value
			peak = pow(peak, tone.x);
		#endif
		peak = peak * CrtsRcpF1(peak * tone.y + tone.z);
		#ifdef CRTS_SATURATION																						// Apply saturation
			ratio = pow(ratio, vec3(tone.w, tone.w, tone.w));
		#endif
		return ratio * peak;																							// Reconstruct color
	#else
		return color;
	#endif
}

void main() {
	vec2 fragCoord = vec2(frag_texture_coord.x, 1.0 - frag_texture_coord.y);
	if(crt_emulation) {
		outcolor.rgb=CrtsFilter(
			fragCoord.xy * resolution,
			src_image_size / resolution,
			src_image_size * vec2(0.5, 0.5),
			1.0 / src_image_size,
			1.0 / resolution,
			2.0 / resolution,
			src_image_size.y,
			// vec2(1.0 / 32.0, 1.0 / 24.0), // this is 'warp' values   1/x*(4/3), 1/x
			vec2(1.0 / 24.0, 1.0 / 16.0),
			INPUT_THIN,
			INPUT_BLUR,
			INPUT_MASK,
			tone_data);
		outcolor = vec4(outcolor.rgb, 1.0) * brightness;
	} else {
		outcolor = texture(iChannel0, fragCoord);
	}
}
