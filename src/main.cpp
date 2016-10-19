#include <core/manager.hpp>
#include <render/render.hpp>
#include <world/map.hpp>
#include <input/input.hpp>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <cstdlib>

Manager manager{};

sf::RenderWindow* window;
RenderSystem* renderSystem;
InputSystem* inputSystem;

bool quit;

void update(sf::Time delta) {

	sf::Event event;
	while(window->pollEvent(event)) {
		if(event.type == sf::Event::Closed)
			quit = true;
	}

	renderSystem->update(delta);
	inputSystem->update(delta);
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

	std::string a;
	sf::RenderWindow window_(sf::VideoMode(800, 600), "Test");
	RenderSystem renderSystem_(manager, window_);
	InputSystem inputSystem_(manager);
	
	window = &window_;
	renderSystem = &renderSystem_;
	inputSystem = &inputSystem_;

	Map myAwesomeMap(manager);
	myAwesomeMap.loadFromFile("assets/maps/untitled.tmx");
	
	quit = false;

	loop();
	window_.close();

	return EXIT_SUCCESS;

}
