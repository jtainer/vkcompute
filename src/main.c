#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "shaders.h"

int main() {

	const char* requestedLayers[] = {
		"VK_LAYER_KHRONOS_validation"
	};
	const uint32_t requestedLayerCount = sizeof(requestedLayers) / sizeof(*requestedLayers);

	// Get supported layer count
	uint32_t supportedLayerCount = 0;
	VkResult result = vkEnumerateInstanceLayerProperties(&supportedLayerCount, NULL);
	if (result != VK_SUCCESS) {
		puts("Failed to get supported layers");
		exit(1);
	}

	VkLayerProperties* supportedLayers = malloc(sizeof(VkLayerProperties) * supportedLayerCount);
	result = vkEnumerateInstanceLayerProperties(&supportedLayerCount, supportedLayers);
	if (result != VK_SUCCESS) {
		puts("Failed to get supported layers");
		exit(1);
	}

	const char** enabledLayers = malloc(sizeof(char*) * requestedLayerCount);
	uint32_t enabledLayerCount = 0;
	for (uint32_t i = 0; i < requestedLayerCount; ++i) {
		for (uint32_t j = 0; j < supportedLayerCount; ++j) {
			if (!strcmp(requestedLayers[i], supportedLayers[j].layerName)) {
				enabledLayers[enabledLayerCount] = requestedLayers[i];
				++enabledLayerCount;
				break;
			}
		}
	}

	free(supportedLayers);

	printf("Enabled layer count: %u\n", enabledLayerCount);
	for (uint32_t i = 0; i < enabledLayerCount; ++i) {
		printf("\t%s\n", enabledLayers[i]);
	}

	// Create a Vulkan instance
	VkInstanceCreateInfo instanceInfo = { 0 };
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pNext = NULL;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = NULL;
	instanceInfo.enabledLayerCount = enabledLayerCount;
	instanceInfo.ppEnabledLayerNames = enabledLayers;
	instanceInfo.enabledExtensionCount = 0;
	instanceInfo.ppEnabledExtensionNames = NULL;

	VkInstance instance = VK_NULL_HANDLE;
	result = vkCreateInstance(&instanceInfo, NULL, &instance);
	if (result != VK_SUCCESS) {
		puts("Failed to create a Vulkan instance");
		exit(1);
	}
	puts("Created a Vulkan instance");

	free(enabledLayers);

	// Query the number of physical devices
	uint32_t physicalDeviceCount = 0;
	result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, NULL);
	if (result != VK_SUCCESS) {
		puts("Failed to enumerate physical devices");
		exit(1);
	}

	// Get handles to each physical devices
	VkPhysicalDevice* physicalDevices = malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);
	result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices);
	if (result != VK_SUCCESS) {
		puts("Failed to enumerate physical devices");
		exit(1);
	}
	
	// Get the properties of each physical device
	puts("Available devices:");
	for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
		VkPhysicalDeviceProperties deviceProperties = { 0 };
		vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);
		printf("\tGPU %u: %s\n", i, deviceProperties.deviceName);
	}

	// Select a discrete GPU
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties physicalDeviceProperties = { 0 };
	for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
		vkGetPhysicalDeviceProperties(physicalDevices[i], &physicalDeviceProperties);
		if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			physicalDevice = physicalDevices[i];
			break;
		}
	}

	// Default to integrated GPU if discrete GPU is not available
	if (physicalDevice == VK_NULL_HANDLE) {
		for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
			VkPhysicalDeviceProperties deviceProperties = { 0 };
			vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);
			if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
				physicalDevice = physicalDevices[i];
				physicalDeviceProperties = deviceProperties;
				break;
			}
		}
	}

	free(physicalDevices);

	if (physicalDevice == VK_NULL_HANDLE) {
		puts("No GPUs found!");
		exit(1);
	}
	else if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		printf("Selected discrete GPU: %s\n", physicalDeviceProperties.deviceName);
	}
	else {
		printf("Selected integrated GPU: %s\n", physicalDeviceProperties.deviceName);
	}

	// Query the number of queue families available for this device
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);
	
	// Get the properties of all available queue families
	VkQueueFamilyProperties* queueFamilyProperties = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties);
	
	// Select find queue families that are capable of compute and transfers
	uint32_t computeQueueIndex = 0;
	uint32_t transferQueueIndex = 0;
	for (uint32_t i = 0; i < queueFamilyCount; ++i) {
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
			computeQueueIndex = i;
		}
	}
	for (uint32_t i = 0; i < queueFamilyCount; ++i) {
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
			transferQueueIndex = i;
		}
	}

	free(queueFamilyProperties);

	printf("Using queue family %u for compute\n", computeQueueIndex);
	printf("Using queue family %u for transfers\n", transferQueueIndex);

	// When we create the logical device, we need to tell it how many queues to create from each queue family
	uint32_t queueCount = 0;
	if (computeQueueIndex == transferQueueIndex) {
		queueCount = 1;
	}
	else {
		queueCount = 2;
	}

	uint32_t queueFamilyIndices[2] = { computeQueueIndex, transferQueueIndex };
	float queuePriorities[] = { 1.f };
	VkDeviceQueueCreateInfo queueCreateInfo[2] = { 0 };
	for (uint32_t i = 0; i < queueCount; ++i) {
		queueCreateInfo[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo[i].pNext = NULL;
		queueCreateInfo[i].flags = 0;
		queueCreateInfo[i].queueFamilyIndex = queueFamilyIndices[i];
		queueCreateInfo[i].queueCount = 1;
		queueCreateInfo[i].pQueuePriorities = queuePriorities;
	}

	// Create the logical device
	VkDeviceCreateInfo deviceCreateInfo = { 0 };
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = NULL;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = queueCount;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfo;
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = NULL;
	deviceCreateInfo.enabledExtensionCount = 0;
	deviceCreateInfo.ppEnabledExtensionNames = NULL;
	deviceCreateInfo.pEnabledFeatures = NULL;

	VkDevice device = VK_NULL_HANDLE;
	result = vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device);
	if (result != VK_SUCCESS) {
		puts("Failed to create a logical device!");
		exit(1);
	}
	else {
		puts("Created a logical device");
	}

	VkQueue computeQueue = VK_NULL_HANDLE;
	VkQueue transferQueue = VK_NULL_HANDLE;
	vkGetDeviceQueue(device, computeQueueIndex, 0, &computeQueue);
	vkGetDeviceQueue(device, transferQueueIndex, 0, &transferQueue);
 
	// Create buffers
	const uint64_t numElements = 256;
	const uint64_t bufferSize = numElements * sizeof(float);
	VkBufferCreateInfo bufferCreateInfo = { 0 };
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = NULL;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.size = bufferSize;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.queueFamilyIndexCount = 1;
	bufferCreateInfo.pQueueFamilyIndices = &computeQueueIndex;
	
	VkBuffer inputBuffer = VK_NULL_HANDLE;
	VkBuffer outputBuffer = VK_NULL_HANDLE;

	result = vkCreateBuffer(device, &bufferCreateInfo, NULL, &inputBuffer);
	if (result != VK_SUCCESS) {
		puts("Failed to create input buffer");
		exit(1);
	}
	result = vkCreateBuffer(device, &bufferCreateInfo, NULL, &outputBuffer);
	if (result != VK_SUCCESS) {
		puts("Failed to create input buffer");
		exit(1);
	}
	printf("Created input and output buffers of size %lu\n", bufferSize);

	// Select a memory heap to allocate from
	VkMemoryRequirements inputBufferMemoryRequirements = { 0 };
	VkMemoryRequirements outputBufferMemoryRequirements = { 0 };
	vkGetBufferMemoryRequirements(device, inputBuffer, &inputBufferMemoryRequirements);
	vkGetBufferMemoryRequirements(device, outputBuffer, &outputBufferMemoryRequirements);

	VkPhysicalDeviceMemoryProperties deviceMemoryProperties = { 0 };
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);
	
	uint32_t memoryTypeIndex = 0;
	uint64_t memoryHeapSize = 0;
	for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; ++i) {
		VkMemoryType memoryType = deviceMemoryProperties.memoryTypes[i];
		if (memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT &&
			memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
			
			memoryTypeIndex = i;
			memoryHeapSize = deviceMemoryProperties.memoryHeaps[memoryType.heapIndex].size;
			break;
		}
	}
	printf("Selected memory heap %u (%lu bytes)\n", memoryTypeIndex, memoryHeapSize);

	// Allocate memory for each buffer
	VkMemoryAllocateInfo inputBufferMemoryAllocateInfo = { 0 };
	VkMemoryAllocateInfo outputBufferMemoryAllocateInfo = { 0 };

	inputBufferMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	inputBufferMemoryAllocateInfo.pNext = NULL;
	inputBufferMemoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
	inputBufferMemoryAllocateInfo.allocationSize = inputBufferMemoryRequirements.size;

	outputBufferMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	outputBufferMemoryAllocateInfo.pNext = NULL;
	outputBufferMemoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
	outputBufferMemoryAllocateInfo.allocationSize = outputBufferMemoryRequirements.size;

	VkDeviceMemory inputBufferMemory = VK_NULL_HANDLE;
	VkDeviceMemory outputBufferMemory = VK_NULL_HANDLE;

	result = vkAllocateMemory(device, &inputBufferMemoryAllocateInfo, NULL, &inputBufferMemory);
	if (result != VK_SUCCESS) {
		puts("Failed to allocate memory for input buffer");
		exit(1);
	}
	result = vkAllocateMemory(device, &outputBufferMemoryAllocateInfo, NULL, &outputBufferMemory);
	if (result != VK_SUCCESS) {
		puts("Failed to allocate memory for output buffer");
		exit(1);
	}

	// Get pointers mapped to device memory
	float* inputBufferMappedPtr = NULL;
	float* outputBufferMappedPtr = NULL;
	result = vkMapMemory(device, inputBufferMemory, 0, bufferSize, 0, (void**) &inputBufferMappedPtr);
	if (result != VK_SUCCESS) {
		puts("Failed to map input buffer memory");
		exit(1);
	}
	result = vkMapMemory(device, outputBufferMemory, 0, bufferSize, 0, (void**) &outputBufferMappedPtr);
	if (result != VK_SUCCESS) {
		puts("Failed to map output buffer memory");
		exit(1);
	}

	// Bind memory to buffers
	result = vkBindBufferMemory(device, inputBuffer, inputBufferMemory, 0);
	if (result != VK_SUCCESS) {
		puts("Failed to bind memory to input buffer");
		exit(1);
	}
	result = vkBindBufferMemory(device, outputBuffer, outputBufferMemory, 0);
	if (result != VK_SUCCESS) {
		puts("Failed to bind memory to output buffer");
		exit(1);
	}

	// Load shader
	const char* shaderFile = "shaders/double.spv";
	VkShaderModule shaderModule = VK_NULL_HANDLE;
	result = LoadShader(device, shaderFile, &shaderModule);
	if (result != VK_SUCCESS) {
		printf("Failed to load shader from file %s\n", shaderFile);
		exit(1);
	}
	printf("Loaded shader from file %s\n", shaderFile);

	// 
	// Do work here
	//

	// Unload shader
	vkDestroyShaderModule(device, shaderModule, NULL);
	puts("Destroyed shader module");

	// Unmap memory
	vkUnmapMemory(device, inputBufferMemory);
	vkUnmapMemory(device, outputBufferMemory);
	puts("Unmapped buffer memory");

	// Free buffer memory
	vkFreeMemory(device, inputBufferMemory, NULL);
	vkFreeMemory(device, outputBufferMemory, NULL);
	puts("Freed buffer memory");

	// Destroy buffers
	vkDestroyBuffer(device, inputBuffer, NULL);
	vkDestroyBuffer(device, outputBuffer, NULL);
	puts("Destroyed input and output buffers");

	// Destroy the logical device
	vkDeviceWaitIdle(device);
	vkDestroyDevice(device, NULL);
	puts("Destroyed the logical device");

	vkDestroyInstance(instance, NULL);
	puts("Destroyed the Vulkan instance");

	return 0;
}
