#include <ui/shapes/FilledRect.hpp>
#include <volk.h>

namespace vanadium::ui::shapes {

	FilledRectShapeRegistry::FilledRectShapeRegistry(UISubsystem*, const graphics::RenderContext& context,
													 VkRenderPass uiRenderPass,
													 const graphics::RenderPassSignature& uiRenderPassSignature)
		: m_rectPipelineID(context.pipelineLibrary->findGraphicsPipeline("UI Filled Rect")),
		  m_dataManager(context, m_rectPipelineID) {
		m_context = context;
		context.pipelineLibrary->createForPass(uiRenderPassSignature, uiRenderPass, { m_rectPipelineID });
	}

	void FilledRectShapeRegistry::addShape(Shape* shape) {
		FilledRectShape* rectShape = reinterpret_cast<FilledRectShape*>(shape);
		m_shapes.push_back(rectShape);
		m_dataManager.addShapeData(m_context, shape->layerIndex(),
								   { .position = rectShape->position(),
									 .size = rectShape->size(),
									 .color = rectShape->color(),
									 .cosSinRotation = { cosf(rectShape->rotation()), sinf(rectShape->rotation()) } });
		m_maxLayer = std::max(m_maxLayer, shape->layerIndex());
	}

	void FilledRectShapeRegistry::removeShape(Shape* shape) {
		auto iterator = std::find(m_shapes.begin(), m_shapes.end(), reinterpret_cast<FilledRectShape*>(shape));
		if (iterator != m_shapes.end()) {
			m_shapes.erase(iterator);
			m_dataManager.eraseShapeData(iterator - m_shapes.begin());
		}
	}

	void FilledRectShapeRegistry::prepareFrame(uint32_t frameIndex) {
		size_t shapeIndex = 0;
		for (auto& shape : m_shapes) {
			m_dataManager.updateShapeData(shapeIndex,
										  { .position = shape->position(),
											.size = shape->size(),
											.color = shape->color(),
											.cosSinRotation = { cosf(shape->rotation()), sinf(shape->rotation()) } });
			++shapeIndex;
		}
		m_dataManager.prepareFrame(m_context, frameIndex);
	}

	void FilledRectShapeRegistry::renderShapes(VkCommandBuffer commandBuffer, uint32_t frameIndex, uint32_t layerIndex,
											   const graphics::RenderPassSignature& uiRenderPassSignature) {
		auto layer = m_dataManager.layer(layerIndex);
		if (layer.elementCount == 0)
			return;
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
						  m_context.pipelineLibrary->graphicsPipeline(m_rectPipelineID, uiRenderPassSignature));
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
								m_context.pipelineLibrary->graphicsPipelineLayout(m_rectPipelineID), 0, 1,
								&m_dataManager.frameDescriptorSet(frameIndex), 0, nullptr);
		VkViewport viewport = { .width = static_cast<float>(m_context.targetSurface->properties().width),
								.height = static_cast<float>(m_context.targetSurface->properties().height),
								.minDepth = 0.0f,
								.maxDepth = 1.0f };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		
		PushConstantData constantData = { .targetDimensions = Vector2(m_context.targetSurface->properties().width,
																	  m_context.targetSurface->properties().height),
										  .instanceOffset = layer.offset };
		VkShaderStageFlags stageFlags =
			m_context.pipelineLibrary->graphicsPipelinePushConstantRanges(m_rectPipelineID)[0].stageFlags;
		vkCmdPushConstants(commandBuffer, m_context.pipelineLibrary->graphicsPipelineLayout(m_rectPipelineID),
						   stageFlags, 0, sizeof(PushConstantData), &constantData);

		vkCmdDraw(commandBuffer, static_cast<uint32_t>(6U * layer.elementCount), 1, 0, 0);
	}

	void FilledRectShapeRegistry::destroy(const graphics::RenderPassSignature&) {
		for (auto& shape : m_shapes) {
			delete shape;
		}
		m_dataManager.destroy(m_context);
	}

	void FilledRectShape::setSize(const Vector2& size) {
		m_size = size;
		m_dirtyFlag = true;
	}

	void FilledRectShape::setColor(const Vector4& color) {
		m_color = color;
		m_dirtyFlag = true;
	}

} // namespace vanadium::ui::shapes