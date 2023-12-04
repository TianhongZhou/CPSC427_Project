#version 330

uniform sampler2D screen_texture;
uniform float time;
uniform float screen_darken_factor;
//uniform int flicker;

in vec2 texcoord;

layout(location = 0) out vec4 color;

vec2 distort(vec2 uv) 
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE THE WATER WAVE DISTORTION HERE (you may want to try sin/cos)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	return uv;
}

const mat4 ditherMatrixFourbyFour = mat4(
    0.0,  8.0,  2.0, 10.0,
   12.0,  4.0, 14.0,  6.0,
    3.0, 11.0,  1.0,  9.0,
   15.0,  7.0, 13.0,  5.0
)/ 16.0; // now pre-computed

const mat2 ditherMatrixTwobyTwo = mat2(
    0.0,  2.0,  
    3.0,  1.0
);



void main() {


    //applying color downsampling and ordered dithering

    float spread = 0.2f;

    float colorCount = 4.0f; // downsampling is kind of weird atm

    vec3 pixelColor = texture(screen_texture, texcoord).xyz;


    int x = int(gl_FragCoord.x) % 4; // finding where the pixel is in the matrix
    int y = int(gl_FragCoord.y) % 4;
    float ditherValue = ditherMatrixFourbyFour[x][y];

    vec3 beforeColorDownsample = pixelColor + ditherValue * spread;

    vec3 downsampled;

    downsampled.r = (floor(beforeColorDownsample.r*(colorCount - 1.0) + 0.5)/(colorCount - 1.0)); 
    downsampled.g = (floor(beforeColorDownsample.g*(colorCount - 1.0) + 0.5)/(colorCount - 1.0));
    downsampled.b = (floor(beforeColorDownsample.b*(colorCount - 1.0) + 0.5)/(colorCount - 1.0));

    color = vec4(downsampled,1.0);

}