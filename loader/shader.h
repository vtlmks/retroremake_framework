//==============================================================
//                      SETUP FOR CRTS
//==============================================================
#define CRTS_TONE 1
#define CRTS_CONTRAST 1
#define CRTS_SATURATION 1
//--------------------------------------------------------------
#define CRTS_WARP 1
//--------------------------------------------------------------
// Try different masks
// #define CRTS_MASK_GRILLE 1
// #define CRTS_MASK_GRILLE_LITE 1
// #define CRTS_MASK_NONE 1
#define CRTS_MASK_SHADOW 1
// --------------------------------------------------------------
// Scanline thinness
//  0.50 = fused scanlines
//  0.70 = recommended default
//  1.00 = thinner scanlines (too thin)
#define INPUT_THIN 0.7
//--------------------------------------------------------------
// Horizonal scan blur
//  -3.0 = pixely
//  -2.5 = default
//  -2.0 = smooth
//  -1.0 = too blurry
#define INPUT_BLUR -2.5
//--------------------------------------------------------------
// Shadow mask effect, ranges from,
//  0.25 = large amount of mask (not recommended, too dark)
//  0.50 = recommended default
//  1.00 = no shadow mask
#define INPUT_MASK 0.5

