#shader vertex
#version 460 core

layout(location = 0) in vec3 pos;

void main()
{
    gl_Position = vec4(pos, 1.0);
}

#shader geometry
#version 460

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out; 



layout(location = 0) out vec3 _pos;


void main() {


    for(int i = 0; i < 6; i++) {

        gl_Layer = i;
		
        for (int j = 0; j < 3; j++) {
            _pos =gl_in[j].gl_Position.xyz; 
            gl_Position = gl_in[j].gl_Position; 
            EmitVertex();
        }
        EndPrimitive(); 

    }
}

#shader fragment
#version 460 core

#define PI 3.1415926535897932384626433832795

layout(location = 0) in vec3 _pos;

layout(set = 0, binding = 1) uniform CaptureData{
    mat4 proj;
	mat4 views[6];
} capture;

layout(location = 0) out vec4 li;

layout(set = 0, binding = 0) uniform samplerCube u_envMap;


void main()
{		

    vec4 worldPos = vec4(_pos,1.0); 
    vec4 viewProjPos = capture.proj * capture.views[gl_Layer] * worldPos;
    vec3 n = normalize(viewProjPos.xyz);

    vec3 irradiance = vec3(0.0);   
    
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, n));
    up         = normalize(cross(n, right));
       
    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * n; 

            irradiance += texture(u_envMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
    li = vec4(irradiance, 1.0);
}