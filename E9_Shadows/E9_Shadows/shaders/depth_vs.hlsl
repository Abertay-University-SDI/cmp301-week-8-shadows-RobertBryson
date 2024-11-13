cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer TimeBuffer : register(b1)
{
    float type;
    float3 padding;
    
    float time;
    float amplitude;
    float frequency;
    float speed;
};

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float4 depthPosition : TEXCOORD0;
};

float4 calcPosDef(float4 inPos)
{
    float4 position = mul(inPos, worldMatrix);
    position = mul(position, viewMatrix);
    position = mul(position, projectionMatrix);
    
    return position;
}

float4 calcPosWave(float4 inPos)
{
    float4 position;
    inPos.y += sin(inPos.x / frequency + time * speed) * amplitude;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    position = mul(inPos, worldMatrix);
    position = mul(position, viewMatrix);
    position = mul(position, projectionMatrix);
    
    return position;
}

OutputType main(InputType input)
{
    OutputType output;

    switch (type)
    {
        case 0: //Flat
            output.position = calcPosDef(input.position);
            break;
        case 1: //Wave
            output.position = calcPosWave(input.position);
            break;
        
        default:
            break;
    }
        
    // Calculate the position of the vertex against the world, view, and projection matrices.
    //output.position = calcPosDef(input.position);

    // Store the position value in a second input value for depth value calculations.
    output.depthPosition = output.position;
	
    return output;
}