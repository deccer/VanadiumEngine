#include <execution>
#include <fstream>
#include <graphics/helper/ErrorHelper.hpp>
#include <graphics/pipelines/PipelineLibrary.hpp>
#include <helper/WholeFileReader.hpp>
#include <volk.h>

namespace vanadium::graphics {
	void PipelineLibrary::create(const std::string& libraryFileName, DeviceContext* context) {
		m_deviceContext = context;

		m_buffer = readFile(libraryFileName.c_str(), &m_fileSize);
		assertFatal(m_fileSize > 0, "PipelineLibrary: Invalid pipeline library file!\n");
		uint64_t headerReadOffset;
		uint32_t version = readBuffer<uint32_t>(headerReadOffset);

		assertFatal(version == pipelineFileVersion, "PipelineLibrary: Invalid pipeline file version!\n");

		uint32_t pipelineCount = readBuffer<uint32_t>(headerReadOffset);
		uint32_t setLayoutCount = readBuffer<uint32_t>(headerReadOffset);

		std::vector<uint64_t> setLayoutOffsets;
		setLayoutOffsets.reserve(setLayoutCount);
		for (uint32_t i = 0; i < setLayoutCount; ++i) {
			setLayoutOffsets.push_back(readBuffer<uint64_t>(headerReadOffset));
		}

		for (auto& offset : setLayoutOffsets) {
			uint32_t bindingCount = readBuffer<uint32_t>(offset);
			std::vector<VkDescriptorSetLayoutBinding> bindings;
			bindings.reserve(bindingCount);

			for (uint32_t i = 0; i < bindingCount; ++i) {
				VkDescriptorSetLayoutBinding binding = { .binding = readBuffer<uint32_t>(offset),
														 .descriptorCount = readBuffer<uint32_t>(offset),
														 .descriptorType = readBuffer<VkDescriptorType>(offset),
														 .stageFlags = readBuffer<VkShaderStageFlags>(offset) };
				uint32_t immutableSamplerCount = readBuffer<uint32_t>(offset);
				if (immutableSamplerCount > 0) {
					for (uint32_t j = 0; j < immutableSamplerCount; ++j) {
						VkSamplerCreateInfo createInfo = { .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
														   .magFilter = readBuffer<VkFilter>(offset),
														   .minFilter = readBuffer<VkFilter>(offset),
														   .mipmapMode = readBuffer<VkSamplerMipmapMode>(offset),
														   .addressModeU = readBuffer<VkSamplerAddressMode>(offset),
														   .addressModeV = readBuffer<VkSamplerAddressMode>(offset),
														   .addressModeW = readBuffer<VkSamplerAddressMode>(offset),
														   .mipLodBias = readBuffer<float>(offset),
														   .anisotropyEnable = readBuffer<bool>(offset),
														   .maxAnisotropy = readBuffer<float>(offset),
														   .compareEnable = readBuffer<bool>(offset),
														   .compareOp = readBuffer<VkCompareOp>(offset),
														   .minLod = readBuffer<float>(offset),
														   .maxLod = readBuffer<float>(offset),
														   .borderColor = readBuffer<VkBorderColor>(offset),
														   .unnormalizedCoordinates = readBuffer<bool>(offset) };
						VkSampler sampler;
						verifyResult(vkCreateSampler(m_deviceContext->device(), &createInfo, nullptr, &sampler));
						m_immutableSamplers.push_back(sampler);
					}
					binding.pImmutableSamplers =
						m_immutableSamplers.data() + m_immutableSamplers.size() - immutableSamplerCount;
				}
			}

			VkDescriptorSetLayoutCreateInfo createInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
														   .bindingCount = bindingCount,
														   .pBindings = bindings.data() };
			VkDescriptorSetLayout layout;
			verifyResult(vkCreateDescriptorSetLayout(m_deviceContext->device(), &createInfo, nullptr, &layout));
			m_descriptorSetLayouts.push_back(layout);
		}

		headerReadOffset = setLayoutOffsets.back();

		std::vector<uint64_t> pipelineDataOffsets;
		pipelineDataOffsets.reserve(pipelineCount);
		for (size_t i = 0; i < pipelineCount; ++i) {
			pipelineDataOffsets.push_back(readBuffer<uint64_t>(headerReadOffset));
		}

		std::mutex pipelineWriteMutex;

		std::for_each(std::execution::par_unseq, pipelineDataOffsets.begin(), pipelineDataOffsets.end(),
					  [this, &pipelineWriteMutex](auto& pipelineOffset) {
						  PipelineType pipelineType = readBuffer<PipelineType>(pipelineOffset);
						  switch (pipelineType) {
							  case PipelineType::Graphics:
								  createGraphicsPipeline(pipelineOffset, pipelineWriteMutex);
								  break;
							  case PipelineType::Compute:
								  createComputePipeline(pipelineOffset, pipelineWriteMutex);
								  break;
						  }
					  });
	}

	void PipelineLibrary::createGraphicsPipeline(uint64_t& bufferOffset, std::mutex& pipelineWriteMutex) {
		uint32_t shaderCount = readBuffer<uint32_t>(bufferOffset);
		std::vector<VkPipelineShaderStageCreateInfo> stageCreateInfos;
		stageCreateInfos.reserve(shaderCount);

		for (uint32_t i = 0; i < shaderCount; ++i) {
			VkShaderStageFlags stageFlags = readBuffer<uint32_t>(bufferOffset);
			uint32_t shaderSize = readBuffer<uint32_t>(bufferOffset);
			assertFatal(bufferOffset + shaderSize < m_fileSize, "PipelineLibrary: Invalid pipeline file version!\n");
			VkShaderModuleCreateInfo createInfo = { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
													.codeSize = shaderSize,
													.pCode = reinterpret_cast<uint32_t*>(
														reinterpret_cast<uintptr_t>(m_buffer) + bufferOffset) };
			VkShaderModule shaderModule;
			verifyResult(vkCreateShaderModule(m_deviceContext->device(), &createInfo, nullptr, &shaderModule));

			bufferOffset = shaderSize;

			stageCreateInfos.push_back({ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
										 .module = shaderModule,
										 .pName = "main" });
		}

		uint32_t setLayoutCount = readBuffer<uint32_t>(bufferOffset);
		std::vector<VkDescriptorSetLayout> setLayouts;
		setLayouts.reserve(setLayoutCount);

		for (uint32_t i = 0; i < setLayoutCount; ++i) {
			setLayouts.push_back(m_descriptorSetLayouts[readBuffer<uint32_t>(bufferOffset)]);
		}

		uint32_t pushConstantRangeCount = readBuffer<uint32_t>(bufferOffset);
		std::vector<VkPushConstantRange> pushConstantRanges;
		pushConstantRanges.reserve(pushConstantRangeCount);

		for (uint32_t i = 0; i < pushConstantRangeCount; ++i) {
			pushConstantRanges.push_back({ .stageFlags = readBuffer<VkShaderStageFlags>(bufferOffset),
										   .offset = readBuffer<uint32_t>(bufferOffset),
										   .size = readBuffer<uint32_t>(bufferOffset) });
		}

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
																.setLayoutCount = setLayoutCount,
																.pSetLayouts = setLayouts.data(),
																.pushConstantRangeCount = pushConstantRangeCount,
																.pPushConstantRanges = pushConstantRanges.data() };
		VkPipelineLayout layout;
		verifyResult(vkCreatePipelineLayout(m_deviceContext->device(), &pipelineLayoutCreateInfo, nullptr, &layout));

		uint32_t instanceCount = readBuffer<uint32_t>(bufferOffset);
		std::vector<uint64_t> instanceOffsets;
		std::vector<std::string> instanceNames;
		std::vector<PipelineLibraryInstance> instances;
		instanceOffsets.reserve(instanceCount);

		for (uint32_t i = 0; i < instanceCount; ++i) {
			instanceOffsets.push_back(readBuffer<uint32_t>(bufferOffset));
		}

		for (auto& offset : instanceOffsets) {
			PipelineLibraryInstance instance;

			uint32_t nameSize = readBuffer<uint32_t>(offset);
			std::string name = std::string(nameSize, ' ');
			assertFatal(offset + nameSize < m_fileSize, "PipelineLibrary: Invalid pipeline file version!\n");
			std::memcpy(name.data(), reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_buffer) + offset), nameSize);

			uint32_t vertexInputConfigCount = readBuffer<uint32_t>(offset);
			
		}

		
	}

	void PipelineLibrary::createComputePipeline(uint64_t& bufferOffset, std::mutex& pipelineWriteMutex) {}
} // namespace vanadium::graphics
