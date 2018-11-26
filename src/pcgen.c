#include "global.h"

#include "pcgen.h"
#include "vk_api.h"

#define PCG_DIRTY 0x0001

// create all necessary objects and generate secondary command buffer to execute within subpass

struct PCGen
{
    uint32_t flags;
    uint32_t pcSize;
    void* pushConstants;
    HVkPipeline pipeline;
    //descriptor sets
    Vec3u grid;
};

void pcgExecuteIfDirty(HVulkan vk, HVkCmdBuff cb, struct PCGen* pcg)
{
    if (!(pcg->flags & PCG_DIRTY))
        return;
    /*
     vkCmdBindPipeline(cmdBuff, VK_PIPELINE_BIND_POINT_COMPUTE, app->generatorPipeline);
     vkCmdBindDescriptorSets(cmdBuff, VK_PIPELINE_BIND_POINT_COMPUTE, app->generatorPipelineLayout, 0, 1, &app->generatorDescr, 0, NULL);
     //buffer to writable state
     //vklBufferBarrierCSOutToVSIn
     //
     vkCmdPushConstants(cmdBuff, app->generatorPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, app->generatorSetupLen, app->generatorSetup);
     vkCmdDispatch(cmdBuff, 1, 1, 1);
     */
    
    //vklCmdBindPipeline(vk, cb, pcg->pipeline);
    //vklCmdBindDescriptorSets(vk, cb, pcg->pipeline, pcg->dsCount, pcg->descSet);
    //vklCmdPushConstants(vk, cb, pcg->pipeline, VK_SHADER_STAGE_COMPUTE_BIT, 0, pcg->pcSize, pcg->pushConstants);
    //vklCmdDispatch(vk, cb, pcg->grid);
}
