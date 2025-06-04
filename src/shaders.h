#include <vulkan/vulkan.h>

#ifndef SHADERS_H
#define SHADERS_H

VkResult LoadShader(VkDevice device, const char* filename, VkShaderModule* shader);

#endif
