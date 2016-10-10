#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <core/ecs/configuration.hpp>
#include <core/ecs/manager.hpp>
#include <core/ecs/typelist.hpp>

struct CBackgroundColor
{
	sf::Color color{};
};
class TBackgroundEntity;

using ComponentList = ecs::TypeList<CBackgroundColor>;
using TagList = ecs::TypeList<TBackgroundEntity>;
using Configuration = ecs::Configuration<ComponentList, TagList>;
using Manager = ecs::Manager<Configuration>;

Manager mgr{};

sf::RenderWindow* window;
bool quit;

void update(sf::Time delta) {

	sf::Event event;
	while(window->pollEvent(event)) {
		if(event.type == sf::Event::Closed)
			quit = true;
	}

	mgr.template forEntitiesHaving<TBackgroundEntity>([](auto entity) {
		auto& bgColor = mgr.getComponent<CBackgroundColor>(entity);
		bgColor.color.r += 1;
		bgColor.color.r %= 255;
		bgColor.color.g += 2;
		bgColor.color.g %= 255;
		bgColor.color.b += 3;
		bgColor.color.b %= 255;
	});

}

void render(sf::Time delta) {

	mgr.template forEntitiesHaving<TBackgroundEntity>([](auto entity) {
		auto& bgColor = mgr.getComponent<CBackgroundColor>(entity);
		window->clear(bgColor.color);
	});

	window->display();
}

void loop() {

	sf::Clock clock;
	sf::Time accumulator;

	const auto max_frame_time = sf::milliseconds(100);
	const auto update_time_step = sf::milliseconds(30);

	while(!quit) {
		auto elapsed = clock.restart();

		if(elapsed > max_frame_time)
			elapsed = max_frame_time;

		accumulator += elapsed;
		while(accumulator >= update_time_step) {
			update(update_time_step);

			accumulator -= update_time_step;
		}

		render(accumulator);
	}

}

int main(int argc, char** argv) {

	sf::RenderWindow window_(sf::VideoMode(800, 600), "Test");
	window = &window_;

	auto bgEntity = mgr.createEntity();
	mgr.template addTag<TBackgroundEntity>(bgEntity);
	mgr.template emplaceComponent<CBackgroundColor>(bgEntity);
	
	quit = false;

	loop();
	window_.close();

	return EXIT_SUCCESS;

}
