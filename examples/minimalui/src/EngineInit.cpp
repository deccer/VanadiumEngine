#include <Engine.hpp>
#include <Log.hpp>
#include <cmath>
#include <ui/functionalities/ButtonFunctionality.hpp>
#include <ui/layouts/FreeLayout.hpp>
#include <ui/shapes/DropShadowRect.hpp>
#include <ui/shapes/Rect.hpp>
#include <ui/shapes/Text.hpp>
#include <ui/styles/RectStyle.hpp>

vanadium::ui::shapes::RectShape* shapes[25 * 25];
vanadium::ui::shapes::TextShape* textShape;
vanadium::ui::Control* buttonControl;

void configureEngine(vanadium::EngineConfig& config) {
	config.setAppName("Vanadium Minimal UI");
	config.addStartupFlag(vanadium::EngineStartupFlag::UIOnly);
}

void preFramegraphInit(vanadium::Engine& engine) {}

void mouseButtonOutput(vanadium::ui::UISubsystem* subsystem, vanadium::ui::Control* triggerControl, void* localData,
					   const vanadium::Vector2& absolutePosition, uint32_t buttonID) {
	vanadium::logInfo("Button was clicked!!");
	vanadium::ui::styles::TextRectStyle::setText(triggerControl->shapes(), "I was clicked!");
}

void init(vanadium::Engine& engine) {
	float x = 0.0f;
	for (uint32_t i = 0; i < 25; ++i) {
		float y = 0.0f;
		for (uint32_t j = 0; j < 25; ++j) {
			shapes[i * 25 + j] = engine.uiSubsystem().addShape<vanadium::ui::shapes::RectShape>(
				vanadium::Vector2(x, y), 0, vanadium::Vector2(30.0f, 10.0f), 13.0f,
				vanadium::Vector4(sinf(x * M_PI) * 0.5 + 0.5, cosf(y * M_PI) * 0.5 + 0.5, 0.0f, 1.0f));
			y += 30.0f;
		}
		x += 50.0f;
	}

	engine.uiSubsystem().addShape<vanadium::ui::shapes::DropShadowRectShape>(
		vanadium::Vector2(200, 200), 2, vanadium::Vector2(300.0f, 180.0f), 0.0f, vanadium::Vector2(0.15), 0.65);

	textShape = engine.uiSubsystem().addShape<vanadium::ui::shapes::TextShape>(
		vanadium::Vector2(300, 300), 1, 640.0f, 0.0f, "spin f  a  s  t", 48.0f, 0,
		vanadium::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	engine.uiSubsystem().addShape<vanadium::ui::shapes::TextShape>(
		vanadium::Vector2(400, 300), 2, 640.0f, 0.0f, "bingus", 48.0f, 0, vanadium::Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	engine.uiSubsystem().addShape<vanadium::ui::shapes::TextShape>(vanadium::Vector2(350, 300), 2, 640.0f, 0.0f,
																   "aaaaaaaa aaaa aa a a  a a a a a a ", 48.0f, 0,
																   vanadium::Vector4(1.0f, 0.0f, 0.0f, 1.0f));

	buttonControl = new vanadium::ui::Control(
		vanadium::ui::createParameters<vanadium::ui::styles::TextRectStyle, vanadium::ui::layouts::FreeLayout,
									   vanadium::ui::functionalities::ButtonFunctionality>(
			&engine.uiSubsystem(), engine.uiSubsystem().rootControl(),
			vanadium::ui::ControlPosition(vanadium::ui::PositionOffsetType::TopLeft, vanadium::Vector2(0.2, 0.2)),
			vanadium::Vector2(300, 50),
			{ .text = "I am a button!", .fontID = 0, .fontSize = 24.0f, .color = vanadium::Vector4(0.0f, 0.0f, 0.0f, 1.0f) },
			{ .mouseButtonHandler = mouseButtonOutput }));
}

bool onFrame(vanadium::Engine& engine) {
	float x = 0.0f;
	for (uint32_t i = 0; i < 25; ++i) {
		float y = 0.0f;
		for (uint32_t j = 0; j < 25; ++j) {
			shapes[i * 25 + j]->setPosition(
				vanadium::Vector2(x + sinf(engine.elapsedTime() + x / (x + y)) * 50,
								  y + sinf(engine.elapsedTime() + (y + x) / y + M_PI / 2.0f) * 50));
			shapes[i * 25 + j]->setRotation(shapes[i * 25 + j]->rotation() + 6.0f * engine.deltaTime());
			y += 10.0f;
		}
		x += 30.0f;
	}
	// textShape->setRotation(textShape->rotation() + 38.0f * engine.deltaTime());
	return true;
}

void destroy(vanadium::Engine& engine) {
	delete buttonControl;
}
