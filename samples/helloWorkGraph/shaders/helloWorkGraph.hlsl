GlobalRootSignature globalRS = { "UAV(u0)" };
RWStructuredBuffer<uint> UAV : register(u0); // 16MB byte buffer from global root sig

struct entryRecord
{
    uint gridSize : SV_DispatchGrid; //or uint3? cpu size can not specify uint3
    uint inputTest;
};

struct secNodeInput
{
    uint test;
};

struct thirdNodeInput
{
    //float test;
    //uint test;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
// NodeID is separate from shader name
// change one doesn't automatically change the other
// [NodeID("TestRoot")]
[NodeMaxDispatchGrid(16,1,1)]

[NumThreads(8, 1, 1)]
void rootNode(
    DispatchNodeInputRecord<entryRecord> input,
    [MaxRecords(2)] NodeOutput<secNodeInput>  secNode, // max invoke 2 secNode
    uint globalThreadID : SV_DispatchThreadID, 
    uint localThreadID : SV_GroupIndex
)
{
    // reserve the mem for MaxRecords output
    //GroupNodeOutputRecords<secNodeInput> outRecs = secOutput.GetGroupNodeOutputRecords(2);

    // set the nodes' data
    //outRecs[localThreadID].test = input.Get().inputTest;

    // invoke the second node
    //outRecs.OutputComplete();
}

[Shader("node")]
[NodeLaunch("thread")]
void secNode(
    ThreadNodeInputRecord<secNodeInput> input
)
{
    
}

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(256, 1, 1)]
void thirdNode(
    //[MaxRecords(256)]GroupNodeInputRecords<thirdNodeInput> inputs,
    //uint localThreadID : SV_GroupIndex
)
{
    // we need to know how much work we get 
    // for this thread group
    //uint numRecords = inputs.Count();

    //if(localThreadID > numRecords) return;
}

// there is another input type: EmptyNodeInput
// which indicate that this node doesn't need
// input from other nodes
// you can declare the EmptyNodeInput in two ways:
//
// void ProcessEmptyNodeInput(
//      EmptyNodeInput input
// )
// or
// void ProcessEmptyNodeInput()
