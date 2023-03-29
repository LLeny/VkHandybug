
#include "vk_texture.h"

void VulkanTexture::updateDescriptor()
{
    descriptor.sampler = sampler;
    descriptor.imageView = view;
    descriptor.imageLayout = imageLayout;
}

void VulkanTexture::destroy()
{
    vkDestroyImageView(device, view, nullptr);
    vkDestroyImage(device, image, nullptr);
    if (sampler)
    {
        vkDestroySampler(device, sampler, nullptr);
    }
    vkFreeMemory(device, deviceMemory, nullptr);
}