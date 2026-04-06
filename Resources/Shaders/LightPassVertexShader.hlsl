struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

VSOutput main(uint id : SV_VertexID)
{
    float2 quadVertices[3] =
    {
        float2(-1, -1),
        float2(-1, 3),
        float2(3, -1),
    };

    VSOutput output;
    output.Position = float4(quadVertices[id], 0.0f, 1.0f);
    output.TexCoord = (quadVertices[id] + 1.0f) * 0.5f;
    output.TexCoord.y = -output.TexCoord.y;
    return output;
}


/*
          ^ Y
          |
          |
    (-1,3)+
         |\ 
         | \
         |  \
         |   \
(-1,-1)+------+(3,-1)  --> X

fullscreentriangle (покрывает весьэкран)
*/