#shader vertex
#version 460 core

layout (location = 0) in vec3 pos;

out vec3 _pos;

uniform mat4 u_proj;
uniform mat4 u_view;

void main()
{
    _pos = pos;  
    gl_Position =  u_proj * u_view * vec4(_pos, 1.0);
}

#shader fragment
 #version 460 core

    #define PI 3.1415926535897932384626433832795

    in vec3 _pos;
	out vec4 li;

    uniform samplerCube u_envMap;


void main()
{		
    vec3 n = normalize(_pos);

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