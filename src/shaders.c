#include "shaders.h"
#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <stdio.h>

VkResult LoadShader(VkDevice device, const char* filename, VkShaderModule* shader) {
	FILE* file = fopen(filename, "rb");
	if (file == NULL) {
		printf("Failed to open shader file %s\n", filename);
		return VK_ERROR_UNKNOWN;
	}

	int result = fseek(file, 0, SEEK_END);
	if (result == -1) {
		printf("Failed to read shader file %s\n", filename);
		fclose(file);
		return VK_ERROR_UNKNOWN;
	}

	long file_size = ftell(file);
	if (file_size == -1) {
		printf("Failed to read shader file %s\n", filename);
		fclose(file);
		return VK_ERROR_UNKNOWN;
	}

	result = fseek(file, 0, SEEK_SET);
	if (result == -1) {
		printf("Failed to read shader file %s\n", filename);
		fclose(file);
		return VK_ERROR_UNKNOWN;
	}

	uint32_t* buffer = malloc(file_size);

	size_t bytes_read = fread(buffer, 1, file_size, file);
	if (bytes_read < file_size) {
		printf("Failed to read shader file %s\n", filename);
		fclose(file);
		free(buffer);
		return VK_ERROR_UNKNOWN;
	}

	fclose(file);

	VkShaderModuleCreateInfo shaderCreateInfo = { 0 };
	shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderCreateInfo.pNext = NULL;
	shaderCreateInfo.flags = 0;
	shaderCreateInfo.codeSize = file_size;
	shaderCreateInfo.pCode = buffer;

	VkResult ret_val = vkCreateShaderModule(device, &shaderCreateInfo, NULL, shader);

	free(buffer);

	return ret_val;
}
