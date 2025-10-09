GlobalRootSignature globalRS = { "UAV(u0)" };
RWStructuredBuffer<uint> UAV : register(u0); // 16MB byte buffer from global root sig

struct entryRecord
{
    uint3 DispatchGrid : SV_DispatchGrid;
};

struct secNodeInput
{
};

struct thirdNodeInput
{
};

[Shader("node")]
[NodeLaunch("broadcasting")]
// NodeID is separate from shader name
// change one doesn't automatically change the other
// [NodeID("TestRoot")]
[NodeMaxDispatchGrid(16,1,1)]

[NumThreads(2, 1, 1)]
void rootNode(uint3 DTid : SV_DispatchThreadID, DispatchNodeInputRecord<entryRecord> input)
{
    
}

[Shader("node")]
[NodeLaunch("thread")]
void secNode()
{
    
}

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(2, 1, 1)]
void thirdNode()
{
    
}
