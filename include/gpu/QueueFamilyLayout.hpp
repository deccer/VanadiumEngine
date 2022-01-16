#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

namespace gpu {
	struct QueueFamilyLayout {
		VkQueueFlags queueCapabilityFlags;
		bool shouldSupportPresent;
		size_t queueCount;
	};
} // namespace gpu