#include "global.h"

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

//device_context begin_frame() begins primary command list
//device_context begin_command_list() starts thread-specific command list
//command_list::begin_pass()
//command_list::end_pass()
//device_context end_frame() adds secondary command lists to primary and submits

//renderer begin_frame() kicks off tasks to record command lists
//renderer end_frame() waits for async tasks and does submit/present

namespace Gfx
{
    
    class DeviceContext;
    
    class CommandList
    {
    public:
        void BeginPass();
        void EndPass();
    protected:
		explicit CommandList(const DeviceContext& dc);
		void InitCommandList(bool isPrimary);
		void DestroyCommandList();
        //handle command pool and buffers
        VkCommandBuffer CmdBuff;
		VkCommandPool CmdPool;
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
		PFN_vkCreateCommandPool vkCreateCommandPool;
		PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
		PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
        //handles for vulkan objects
		const VkAllocationCallbacks* Alloc;
		VkDevice Device;
    };
    
}

Gfx::CommandList::CommandList(const Gfx::DeviceContext& dc)
    : DC(dc)
{
}

void Gfx::CommandList::BeginPass()
{
	VkRenderPassBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	//TODO
    DC.vkCmdBeginRenderPass(CmdBuff, &info, VK_SUBPASS_CONTENTS_INLINE);
}

void Gfx::CommandList::EndPass()
{
    DC.vkCmdEndRenderPass(CmdBuff);
}

void Gfx::CommandList::InitCommandList(bool isPrimary)
{
	{
		VkCommandPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		//queue family
		DC.vkCreateCommandPool(DC.Device, &info, DC.Alloc, &CmdPool); // error? 
	}
	//vk->cmd.buffer = memForwdAlloc(vk->mem, vk->scSize * sizeof(VkCommandBuffer));
	{
		VkCommandBufferAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.commandPool = CmdPool;
		info.level = (isPrimary) ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		//info.commandBufferCount = vk->scSize;
		//VKFN(vk->AllocateCommandBuffersImpl(vk->dev, &cbInfo, vk->cmd.buffer)); 
	}
}

void Gfx::CommandList::DestroyCommandList()
{}

Gfx::DeviceContext::DeviceContext()
    : Gfx::CommandList(*this)
{
}
