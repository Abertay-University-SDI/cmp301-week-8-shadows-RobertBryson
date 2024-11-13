
cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	matrix lightViewMatrix[2];
	matrix lightProjectionMatrix[2];
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
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    float4 lightViewPos[2] : TEXCOORD1;
};


float4 calcPosDef(float4 inPos)
{
    float4 position =  mul(inPos, worldMatrix);
    position = mul(position, viewMatrix);
    position = mul(position, projectionMatrix);
    
    return position;
}

float4 calcViewPosDef(float4 inPos, int i)
{
    float4 position = mul(inPos, worldMatrix);
    position = mul(position, lightViewMatrix[i]);
    position = mul(position, lightProjectionMatrix[i]);
    
    return position;
}

OutputType main(InputType input)
{
    OutputType output;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = calcPosDef(input.position);
    
	// Calculate the position of the vertice as viewed by the light source.
    for (int i = 0; i < 2; i++)
    {
        output.lightViewPos[i] = calcViewPosDef(input.position, i);
    }

    output.tex = input.tex;
    output.normal = mul(input.normal, (float3x3)worldMatrix);
    output.normal = normalize(output.normal);

	return output;
}