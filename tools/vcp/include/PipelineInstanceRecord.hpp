#pragma once 

#include <json/json.h>
#include <PipelineStructs.hpp>

#include <ParsingUtils.hpp>
#include <spirv_reflect.h>

class PipelineInstanceRecord {
  public:
	PipelineInstanceRecord(PipelineType type, const std::string_view& srcPath, const Json::Value& instanceNode);

	size_t serializedSize() const;
	void serialize(void* data);

	void verifyInstance(const std::string_view& srcPath, const std::vector<ReflectedShader>& shaderModules);

	bool isValid() const { return m_isValid; }

  private:
	bool m_isValid = true;

	void deserializeVertexInput(const std::string_view& srcPath, const Json::Value& node);
	void deserializeInputAssembly(const std::string_view& srcPath, const Json::Value& node);
	void deserializeRasterization(const std::string_view& srcPath, const Json::Value& node);
	void deserializeMultisample(const std::string_view& srcPath, const Json::Value& node);
	void deserializeDepthStencil(const std::string_view& srcPath, const Json::Value& node);
	VkStencilOpState deserializeStencilState(const std::string_view& srcPath, const Json::Value& node);
	void deserializeColorBlend(const std::string_view& srcPath, const Json::Value& node);
	void deserializeColorAttachmentBlend(const std::string_view& srcPath, const Json::Value& node);
	void deserializeSpecializationConfigs(const std::string_view& srcPath, const Json::Value& node);

	void* serializeStencilState(const VkStencilOpState& state, void* data);

	void verifyVertexShader(const std::string_view& srcPath, const SpvReflectShaderModule& shader);
	void verifyFragmentShader(const std::string_view& srcPath, const SpvReflectShaderModule& shader);

	uint32_t asUIntOr(const Json::Value& value, const std::string_view& name, uint32_t fallback);
	float asFloatOr(const Json::Value& value, const std::string_view& name, float fallback);
	bool asBoolOr(const Json::Value& value, const std::string_view& name, bool fallback);
	const char* asCStringOr(const Json::Value& value, const std::string_view& name, const char* fallback);
	std::string asStringOr(const Json::Value& value, const std::string_view& name, const std::string& fallback);

	std::string m_name;
	PipelineType m_type;

	InstanceVertexInputConfig m_instanceVertexInputConfig;
	InstanceInputAssemblyConfig m_instanceInputAssemblyConfig;
	InstanceRasterizationConfig m_instanceRasterizationConfig;
	InstanceMultisampleConfig m_instanceMultisampleConfig;
	InstanceDepthStencilConfig m_instanceDepthStencilConfig;
	InstanceColorBlendConfig m_instanceColorBlendConfig;
	std::vector<InstanceColorAttachmentBlendConfig> m_instanceColorAttachmentBlendConfigs;

	std::vector<InstanceSpecializationConfig> m_instanceSpecializationConfigs;
	size_t m_currentSpecializationDataSize = 0;
};