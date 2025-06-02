#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <stdio.h>


int main() {

	// Create a Vulkan instance
	VkInstanceCreateInfo instanceInfo = { 0 };
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pNext = NULL;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = NULL;
	instanceInfo.enabledLayerCount = 0;
	instanceInfo.ppEnabledLayerNames = NULL;
	instanceInfo.enabledExtensionCount = 0;
	instanceInfo.ppEnabledExtensionNames = NULL;

	VkInstance instance = VK_NULL_HANDLE;
	VkResult result = vkCreateInstance(&instanceInfo, NULL, &instance);
	if (result != VK_SUCCESS) {
		puts("Failed to create a Vulkan instance");
		exit(1);
	}
	puts("Created a Vulkan instance");

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

	// Destroy the logical device
	vkDestroyDevice(device, NULL);
	puts("Destroyed the logical device");

	vkDestroyInstance(instance, NULL);
	puts("Destroyed the Vulkan instance");

	return 0;
}
