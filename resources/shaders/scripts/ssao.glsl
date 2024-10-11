
float UnsharpSSAO(sampler2D ssaoMap){

    const float kernelDimension = 15.0;
    const ivec2 screenSize = textureSize(ssaoMap,0);

    float occlusion = 0.0;

    int i = int(gl_FragCoord.x);
    int j = int(gl_FragCoord.y);

    int maxX = i + int(floor(kernelDimension*0.5));
    int maxY = j + int(floor(kernelDimension*0.5));

    float sampX;
    float sampY;

    float neighborCount = 0;

    for (int x = i - int(floor(kernelDimension*0.5)); x < maxX; x++) {
    for (int y = j - int(floor(kernelDimension*0.5)); y < maxY; y++) {
    
    sampX = float(x) / screenSize.x;
    sampY = float(y) / screenSize.y;

    if (sampX >= 0.0 && sampX <= 1.0 && sampY >= 0.0 && sampY <= 1.0 &&
    
    abs( linearizeDepth(texture(ssaoMap,gl_FragCoord.xy / screenSize.xy).a,0.1,100.0) -
     linearizeDepth(texture(ssaoMap,vec2(sampX,sampY)).a, 0.1,100.0)) < 0.02) {
    occlusion +=   linearizeDepth(texture(ssaoMap,vec2(sampX,sampY)).a,0.1,100.0);
    neighborCount++;
    }
    }
    }

    occlusion = occlusion / neighborCount;
     
     
    occlusion = 20 * ( linearizeDepth(texture(ssaoMap,gl_FragCoord.xy / screenSize.xy).a, 0.1,100.0) - max(0.0, occlusion));


  return occlusion;

}