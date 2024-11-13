// Light pixel shader
// Calculate diffuse lighting for a single directional light (also texturing)

Texture2D texture0 : register(t0);
Texture2D depthMapTexture[2] : register(t1);

SamplerState sampler0 : register(s0);
SamplerState shadowSampler : register(s1);

struct psLightStr
{
    float4 ambient;
    float4 diffuse;
    float3 direction;
    float padding;
};

cbuffer LightBuffer : register(b0)
{
    psLightStr psLights[2];
}

cbuffer TimeBuffer : register(b1)
{
    float height;
    float3 padding;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float4 lightViewPos[2] : TEXCOORD1;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate(diffuse * intensity);
    return colour;
}

float getHeight(float2 uv)
{
    float h = texture0.SampleLevel(sampler0, uv, 0).r;
    return h * height * 15;
}


float3 calculateNormal(float2 uv)
{
    float texturedimensions = 256.0f;
    
    //float val;
    //texture0.GetDimensions(0, 256, 256, val);
    
    float uvOffset = 0.001f;
    float step = uvOffset * 2 * 100;
    

    float up = getHeight(float2(uv.x, uv.y + uvOffset));
    float down = getHeight(float2(uv.x, uv.y - uvOffset));

    float left = getHeight(float2(uv.x - uvOffset, uv.y));
    float right = getHeight(float2(uv.x + uvOffset, uv.y));

    float3 tangent = normalize(float3(step, right - left, 0));
    float3 biTangent = normalize(float3(0.f, up - down, step));
        
    return cross(biTangent, tangent);
}

// Is the gemoetry in our shadow map
bool hasDepthData(float2 uv)
{
    if (uv.x < 0.f || uv.x > 1.f || uv.y < 0.f || uv.y > 1.f)
    {
        return false;
    }
    return true;
}

bool isInShadow(Texture2D sMap, float2 uv, float4 lightViewPosition, float bias)
{
    // Sample the shadow map (get depth of geometry)
    float depthValue = sMap.Sample(shadowSampler, uv).r;
	// Calculate the depth from the light.
    float lightDepthValue = lightViewPosition.z / lightViewPosition.w;
    lightDepthValue -= bias;

	// Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
    if (lightDepthValue < depthValue)
    {
        return false;
    }
    return true;
}

float2 getProjectiveCoords(float4 lightViewPosition)
{
    // Calculate the projected texture coordinates.
    float2 projTex = lightViewPosition.xy / lightViewPosition.w;
    projTex *= float2(0.5, -0.5);
    projTex += float2(0.5f, 0.5f);
    return projTex;
}

float4 main(InputType input) : SV_TARGET
{
    float shadowMapBias = 0.005f;
    float4 colour[2];
    float4 textureColour = texture0.Sample(sampler0, input.tex);
    float4 colourTot = float4(0.f, 0.f, 0.f, 1.f);

    input.normal = calculateNormal(input.tex);
    
    for (int i = 0; i < 2; i++)
    {
        // Calculate the projected texture coordinates.
        float2 pTexCoord = getProjectiveCoords(input.lightViewPos[i]);
        colour[i] = float4(0.f, 0.f, 0.f, 1.f);
        // Shadow test. Is or isn't in shadow
        if (hasDepthData(pTexCoord))
        {
        // Has depth map data
            if (!isInShadow(depthMapTexture[i], pTexCoord, input.lightViewPos[i], shadowMapBias))
            {
            // is NOT in shadow, therefore light
                colour[i] = calculateLighting(-psLights[i].direction, input.normal, psLights[i].diffuse);
            }
        }
    }
    
    
    colourTot = saturate(colour[0] + colour[1] + psLights[0].ambient);
    return saturate(colourTot) * textureColour;
    
    
    //float4 textureColour;
    //float4 lightColour;
    //float3 norms;
    //
	//// Sample the texture. Calculate light intensity and colour, return light*texture for final pixel colour.
    //float2 uv = input.tex;
    //norms = calculateNormal(uv);
    //textureColour = texture0.Sample(sampler0, uv);
    //lightColour = calculateLighting(-direction, norms, diffuse);
    //
    ////return float4(norms, 0); //shows normals
    //return lightColour * textureColour;
}



