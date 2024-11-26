#shader vertex
#version 460 core
#include object.glsl

//Input
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 uv;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 color;

//Output
layout(location = 0) out vec3 v_color;
layout(location = 1) out vec3 v_tangent;

void main() {

    gl_Position = object.model * vec4(position, 1.0);

    v_tangent = normalize(mat3(transpose(inverse(object.model))) * tangent);
    v_color = color;

}

#shader geometry
#version 460 core
#include camera.glsl

//Setup
layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

//Input
layout(location = 0) in vec3 v_color[];
layout(location = 1) in vec3 v_tangent[];

//Uniforms
layout(set = 1, binding = 1) uniform MaterialUniforms {
    vec3 baseColor;
    float thickness;
} material;

//Output
layout(location = 0) out vec3 g_pos;
layout(location = 1) out vec3 g_modelPos;
layout(location = 2) out vec3 g_normal;
layout(location = 3) out vec3 g_modelNormal;
layout(location = 4) out vec2 g_uv;
layout(location = 5) out vec3 g_dir;
layout(location = 6) out vec3 g_modelDir;
layout(location = 7) out vec3 g_color;
layout(location = 8) out vec3 g_origin;


void emitQuadPoint(
    vec4 origin,
    vec4 right,
    float offset,
    vec3 forward,
    vec3 normal,
    vec2 uv,
    int id
) {

    vec4 newPos = origin + right * offset; //Model space
    gl_Position = camera.viewProj * newPos;
    g_dir = normalize(mat3(transpose(inverse(camera.view))) * v_tangent[id]);
    g_modelDir = v_tangent[id];
    g_color = v_color[id];
    g_pos = (camera.view * newPos).xyz;
    g_modelPos = newPos.xyz;
    g_uv = uv;
    g_normal = normalize(mat3(transpose(inverse(camera.view))) * normal);
    g_modelNormal = normal;
    g_origin = (camera.view * origin).xyz;

    EmitVertex();
}

void main() {

        //Model space --->>>

    vec4 startPoint = gl_in[0].gl_Position;
    vec4 endPoint = gl_in[1].gl_Position;

    vec4 view0 = vec4(camera.position.xyz, 1.0) - startPoint;
    vec4 view1 = vec4(camera.position.xyz, 1.0) - endPoint;

    vec3 dir0 = v_tangent[0];
    vec3 dir1 = v_tangent[1];

    vec4 right0 = normalize(vec4(cross(dir0.xyz, view0.xyz), 0.0));
    vec4 right1 = normalize(vec4(cross(dir1.xyz, view1.xyz), 0.0));

    vec3 normal0 = normalize(cross(right0.xyz, dir0.xyz));
    vec3 normal1 = normalize(cross(right1.xyz, dir1.xyz));

        //<<<----

    float halfLength = material.thickness * 0.5;

    emitQuadPoint(startPoint, right0, halfLength, dir0, normal0, vec2(1.0, 0.0), 0);
    emitQuadPoint(endPoint, right1, halfLength, dir1, normal1, vec2(1.0, 1.0), 1);
    emitQuadPoint(startPoint, -right0, halfLength, dir0, normal0, vec2(0.0, 0.0), 0);
    emitQuadPoint(endPoint, -right1, halfLength, dir1, normal1, vec2(0.0, 1.0), 1);

}

#shader fragment
#version 460 core
#include light.glsl
#include scene.glsl
#include camera.glsl
#include object.glsl
#include utils.glsl
#include shadow_mapping.glsl
#include reindhart.glsl
#include BRDFs/marschner_LUT_BSDF.glsl

//Input
layout(location = 0) in vec3 g_pos;
layout(location = 1) in vec3 g_modelPos;
layout(location = 2) in vec3 g_normal;
layout(location = 3) in vec3 g_modelNormal;
layout(location = 4) in vec2 g_uv;
layout(location = 5) in vec3 g_dir;
layout(location = 6) in vec3 g_modelDir;
layout(location = 7) in vec3 g_color;
layout(location = 8) in vec3 g_origin;

//Uniforms
layout(set = 0, binding = 2) uniform sampler2DArray shadowMap;
layout(set = 0, binding = 4) uniform samplerCube irradianceMap;


layout(set = 1, binding = 1) uniform MaterialUniforms {
    vec3 baseColor;
    float thickness;

    float Rpower;
    float TTpower;
    float TRTpower; 
    float density;

    bool glints;
    bool useScatter;
    bool coloredScatter;
    bool r;

    bool tt;
    bool trt;
    bool occlusion;
} material;

layout(set = 2, binding = 0) uniform sampler2D nTex1;
layout(set = 2, binding = 1) uniform sampler2D nTex2;
layout(set = 2, binding = 2) uniform sampler3D GITex;
layout(set = 2, binding = 3) uniform sampler2D mGITex;
layout(set = 2, binding = 4) uniform sampler2D nGITex1;
layout(set = 2, binding = 5) uniform sampler2D nGITex2;

MarschnerLookupBSDF bsdf;

layout(location = 0) out vec4 fragColor;


vec3 computeAmbient(vec3 n) {

    vec3 ambient;
    if(scene.useIBL){
        float rad = radians(scene.envRotation);
        float c = cos(rad);
        float s = sin(rad);
        mat3 rotationY = mat3(c, 0.0, -s,
                0.0, 1.0, 0.0,
                s, 0.0, c);
        vec3 rotatedNormal = normalize(rotationY * n);

        ambient = evalMarschnerLookupBSDF(
                rotatedNormal, 
                normalize(-g_pos),
                texture(irradianceMap, rotatedNormal).rgb*scene.ambientIntensity,
                bsdf, 
                nTex1,
                nTex2,
                GITex,
                mGITex,
                nGITex1,
                nGITex2,
                vec3(0.5),
                vec3(0.5),
                0.1,
                material.r, 
                false, 
                material.trt,
                material.useScatter);


        // ambient = (scene.ambientIntensity * scene.ambientColor) ;
    }else{
        ambient = (scene.ambientIntensity * scene.ambientColor) ;
    }
    return ambient;
}

void main() {

    //BSDF setup ............................................................
    bsdf.tangent =  normalize(g_dir);
    bsdf.Rpower = material.Rpower;
    bsdf.TTpower = material.TTpower;
    bsdf.TRTpower = material.TRTpower;
    bsdf.density = material.density;


    //DIRECT LIGHTING .......................................................
    vec3 color = vec3(0.0);
    for(int i = 0; i < scene.numLights; i++) {
        //If inside liught area influence
        if(isInAreaOfInfluence(scene.lights[i], g_pos)) {

            vec3    shadow = vec3(1.0);
            vec3    spread = vec3(0.0);
            float   directFraction = 1.0;
            if(int(object.otherParams.y) == 1 && scene.lights[i].shadowCast == 1) {
                if(scene.lights[i].shadowType == 0) //Classic
                    shadow = computeHairShadow(scene.lights[i], i,shadowMap, bsdf.density, g_modelPos,spread, directFraction);
                if(scene.lights[i].shadowType == 1) //VSM   
                    shadow = computeHairShadow(scene.lights[i], i,shadowMap, bsdf.density, g_modelPos,spread, directFraction);
            }
            vec3 lighting = evalMarschnerLookupBSDF(
                normalize(scene.lights[i].position.xyz - g_pos), 
                normalize(-g_pos),
                scene.lights[i].color * scene.lights[i].intensity,
                bsdf, 
                nTex1,
                nTex2,
                GITex,
                mGITex,
                nGITex1,
                nGITex2,
                shadow,
                spread,
                directFraction,
                material.r, 
                material.tt, 
                material.trt,
                material.useScatter);

            color += lighting;
        }
    }


    // vec3 n1 = cross(g_modelDir, cross(camera.position.xyz, g_modelDir));
    vec3 fakeNormal = normalize(g_modelPos-object.volumeCenter);
    // vec3 fakeNormal = mix(n1,n2,0.5);

    //AMBIENT COMPONENT ..........................................................

    vec3 ambient = computeAmbient(fakeNormal);
    color += ambient;

    if(int(object.otherParams.x) == 1 && scene.enableFog) {
        float f = computeFog(gl_FragCoord.z);
        color = f * color + (1 - f) * scene.fogColor.rgb;
    }


    fragColor = vec4(color, 1.0);

}