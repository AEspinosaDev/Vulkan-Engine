#shader vertex
#version 460

layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec2 v_uv;

void main() {
    gl_Position = vec4(pos, 1.0);

    v_uv = uv;
}

#shader fragment
#version 460

layout(location = 0) in  vec2 v_uv;

/*Modify values of set and binding according to needs*/
layout(set = 1, binding = 0) uniform sampler2D inputImage;

layout(push_constant) uniform Settings {
    float radius; 
} settings;


layout(location = 0) out vec4 outputImage;

#define SIGMA 10.0 //A preprocessor macro defining the spatial kernel standard deviation.
#define BSIGMA 1.0 //A preprocessor macro defining the color kernel standard deviation.

float normpdf(in float x, in float sigma)
{
	return 0.39894*exp(-0.5*x*x/(sigma*sigma))/sigma;
}

float normpdf3(in vec4 v, in float sigma)
{
	return 0.39894*exp(-0.5*dot(v,v)/(sigma*sigma))/sigma;
}

void main()
{
    vec2 imgSize = vec2(textureSize(inputImage, 0));
    vec4 c = texture(inputImage, v_uv);

	const int kSize = int(settings.radius);
	float kernel[20];
	vec4 filteredColour = vec4(0.0);
	
	//create the 1-D kernel
	float Z = 0.0;
	for (int j = 0; j <= kSize; ++j)
	{
		kernel[kSize+j] = kernel[kSize-j] = normpdf(float(j), SIGMA);
	}
	
	
	vec4 cc;
	float factor;
	float bZ = 1.0/normpdf(0.0, BSIGMA);
	//read out the texels
	for (int i=-kSize; i <= kSize; ++i)
	{
		for (int j=-kSize; j <= kSize; ++j)
		{
            cc = texture(inputImage, v_uv + vec2(float(i),float(j)) / imgSize.xy);
			factor = normpdf3(cc-c, BSIGMA)*bZ*kernel[kSize+j]*kernel[kSize+i];
			Z += factor;
			filteredColour += factor*cc;
		}
    }       

    outputImage = vec4(filteredColour/Z);
}

