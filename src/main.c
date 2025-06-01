#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <stdio.h>


int main() {
	VkInstanceCreateInfo instanceInfo = { 0 };
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pNext = NULL;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = NULL;
	instanceInfo.enabledLayerCount = 0;
	instanceInfo.ppEnabledLayerNames = NULL;
	instanceInfo.enabledExtensionCount = 0;
	instanceInfo.ppEnabledExtensionNames = NULL;

	VkInstance instance;
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
	for (uint32_t i = 0; i < physicalDeviceCount; i++) {
		VkPhysicalDeviceProperties deviceProperties = { 0 };
		vkGetPhysicalDeviceProperties(physicalDevices[0], &deviceProperties);
		printf("GPU %u: %s\n", i, deviceProperties.deviceName);
	}

	free(physicalDevices);

	vkDestroyInstance(instance, NULL);
	puts("Destroyed the Vulkan instance");

	return 0;
}
