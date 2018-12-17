#include "global.h"

namespace Gfx
{
    
    class DeviceContext;
    
    class CommandList
    {
    public:
        explicit CommandList(const DeviceContext& dc);
        void BeginPass();
        void EndPass();
    protected:
        //handle command pool and buffers
        //VkCommandBuffer CmdBuff; //handle of current buffer
    private:
        const DeviceContext& DC;
    };
    
    class DeviceContext : public CommandList
    {
    public:
        explicit DeviceContext();
        void BeginFrame();
        CommandList& MakeCommandList();
        void ExecuteCommandList(CommandList& cmdList);
        void EndFrame();
    private:
        friend class CommandList;
        //pointers to vulkan functions
        //handles for vulkan objects
    };
    
}

Gfx::CommandList::CommandList(const Gfx::DeviceContext& dc)
    : DC(dc)
{
}

void Gfx::CommandList::BeginPass()
{
    //prepare structure
    //DC.vkCmdBeginRenderPass(CmdBuff, &info, VK_SUBPASS_CONTENTS_INLINE);
}

void Gfx::CommandList::EndPass()
{
    //prepare struct
    //DC.vkEndRenderPass(CmdBuff);
}

Gfx::DeviceContext::DeviceContext()
    : Gfx::CommandList(*this)
{
}
