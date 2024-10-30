vec3 reindhart(vec3 color){
    color = color / (color + vec3(1.0));
    return color;
    // const float GAMMA = 2.2;
    // return pow(color, vec3(1.0 / GAMMA));
}