vec3 reindhartTonemap(vec3 color){
    return color / (color + vec3(1.0));
}
vec3 linear2hdr(vec3 lcolor){
    const float GAMMA = 2.2;
    return pow(lcolor, vec3(1.0 / GAMMA));
}
