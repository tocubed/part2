#include <core/manager.hpp>
#include <render/render.hpp>
#include <world/map.hpp>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <cstdlib>

Manager manager{};
Map myAwesomeMap(manager);

sf::RenderWindow* window;
RenderSystem* renderSystem;

bool quit;

void update(sf::Time delta) {

	sf::Event event;
	while(window->pollEvent(event)) {
		if(event.type == sf::Event::Closed)
			quit = true;
	}

	renderSystem->update(delta);

	auto anotherCircle = manager.createEntity();
	manager.addComponent<CLocation>(anotherCircle, CLocation{0, 0, 1});
	manager.addComponent<CDrawable>(anotherCircle, CDrawable{new sf::CircleShape(3)});

	manager.forEntitiesHaving<CLocation>([](auto entity)
	{
		auto& pos = manager.getComponent<CLocation>(entity);

		pos.x += (std::rand() % 8) - 1;
		pos.y += (std::rand() % 8) - 1;
	});
}

void render(sf::Time delta) {

	window->clear(sf::Color::Black);

	renderSystem->render(delta);

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
	RenderSystem renderSystem_(manager, window_);
	
	window = &window_;
	renderSystem = &renderSystem_;

	myAwesomeMap.loadFromFile("assets/maps/untitled.tmx");

	auto mySquareEntity = manager.createEntity();
	manager.addComponent<CLocation>(mySquareEntity, CLocation{0, 0, 1});
	manager.addComponent<CDrawable>(mySquareEntity, CDrawable{new sf::CircleShape(100, 4)});

	auto myCircleEntity = manager.createEntity();
	manager.addComponent<CLocation>(myCircleEntity, CLocation{0, 0, 1});
	manager.addComponent<CDrawable>(myCircleEntity, CDrawable{new sf::CircleShape(10)});

	
	quit = false;

	loop();
	window_.close();

	return EXIT_SUCCESS;

}
