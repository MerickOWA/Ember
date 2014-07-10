struct GSInput
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
};

struct GSOutput
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
};

[maxvertexcount(3)]
void main(triangle GSInput input[3],
          inout TriangleStream< GSOutput > output)
{
    for (uint i = 0; i < 3; i++)
    {
        GSOutput element;
        element.pos = input[i].pos;
        element.color = input[i].color;
        output.Append(element);
    }
}