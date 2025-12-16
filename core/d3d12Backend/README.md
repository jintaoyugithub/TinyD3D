# TinyD3D Backend

| Class             | Desc.                                                                                   |
| ----------------- | --------------------------------------------------------------------------------------- |
| DescriptorHeap    | addDescriptor(), auto auto manage the descriptor allocation and offset under the hood   |
| GPUResource       | Basic class of all gpu resource such as buffer and texture, including state tracking    |
| Pipelinestate     | addShaderDxc(), addShaderFxc(), createDefaultGraphicsPso(), createDefaultCompPso()      |
| Queue             | executeCmdList(),  waitForFence()                                                       |
| ResourceAllocator | Actual allocation happen, createBuffer(), other feature like CreateCommittedResource3() |
|                   |                                                                                         |