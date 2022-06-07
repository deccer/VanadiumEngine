#pragma once

#include <graphics/RenderContext.hpp>
#include <ui/ShapeRegistry.hpp>
#include <ui/SimpleShapeDataManager.hpp>
#include <vector>

namespace vanadium::ui {
	class UISubsystem;
}

namespace vanadium::ui::shapes {

	class FilledRectShape;

	class FilledRectShapeRegistry : public ShapeRegistry {
	  public:
		FilledRectShapeRegistry(UISubsystem*, const graphics::RenderContext& context, VkRenderPass uiRenderPass,
								const graphics::RenderPassSignature& uiRenderPassSignature);

		void addShape(Shape* shape) override;
		void removeShape(Shape* shape) override;
		void prepareFrame(uint32_t frameIndex) override;
		void renderShapes(VkCommandBuffer commandBuffers, uint32_t frameIndex, uint32_t layerIndex,
						  const graphics::RenderPassSignature& uiRenderPassSignature) override;
		void destroy(const graphics::RenderPassSignature& uiRenderPassSignature) override;

	  private:
		struct ShapeData {
			Vector2 position;
			Vector2 size;
			Vector4 color;
			float cosSinRotation[2];
			float _pad[2];
		};
		struct PushConstantData {
			Vector2 targetDimensions;
			uint32_t instanceOffset;
		};

		uint32_t m_rectPipelineID;
		SimpleShapeDataManager<ShapeData> m_dataManager;

		graphics::RenderContext m_context;
		std::vector<FilledRectShape*> m_shapes;
	};

	class FilledRectShape : public Shape {
	  public:
		using ShapeRegistry = FilledRectShapeRegistry;

		FilledRectShape(Vector2 pos, uint32_t layerIndex, Vector2 size, float rotation, Vector4 color)
			: Shape("Filled Rect", layerIndex, pos, rotation), m_size(size), m_color(color) {}

		const Vector2& size() const { return m_size; }
		const Vector4& color() const { return m_color; }
		void setSize(const Vector2& size);
		void setColor(const Vector4& color);

		static size_t vertexDataSize() { return sizeof(m_vertexData); };
		static size_t indexDataSize() { return sizeof(m_indexData); }
		static void writeVertexData(void* data) { std::memcpy(data, m_vertexData, sizeof(m_vertexData)); }
		static void writeIndexData(void* data) { std::memcpy(data, m_indexData, sizeof(m_indexData)); }

	  private:
		static constexpr Vector2 m_vertexData[] = { { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f } };
		static constexpr uint32_t m_indexData[] = { 0, 2, 3, 3, 1, 0 };

		Vector2 m_size;
		Vector4 m_color;
	};

} // namespace vanadium::ui::shapes