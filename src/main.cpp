#include <core/manager.hpp>
#include <input/input.hpp>
#include <render/animation.hpp>
#include <render/character.hpp>
#include <render/render.hpp>
#include <world/movement.hpp>
#include <world/overworld.hpp>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <cstdlib>
#include <sstream>
#include <string>

Manager manager;
Overworld overworld(manager, 30);

sf::RenderWindow* window;

InputSystem* inputSystem;
MovementSystem* movementSystem;
RenderSystem* renderSystem;
AnimationSystem* animationSystem;

bool quit;

void update(sf::Time delta) {

	sf::Event event;
	while(window->pollEvent(event)) {
		if(event.type == sf::Event::Closed)
			quit = true;
	}

	inputSystem->update(delta);
	movementSystem->update(delta);
	animationSystem->update(delta);
	renderSystem->update(delta);

	// TODO Move event cleanup elsewhere
	manager.forEntitiesHaving<TEvent>([](EntityIndex eI)
	{
		manager.deleteEntity(eI);
	});
}

void render(sf::Time delta) {

	window->clear(sf::Color::Black);

	renderSystem->render(delta);

	window->display();
}

unsigned int fpsCounter;
sf::Time fpsTimer;
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

		// FPS Counter
		fpsTimer += elapsed;
		fpsCounter++;

		if(fpsTimer >= sf::milliseconds(1000))
		{
			fpsTimer = sf::milliseconds(0);

			std::stringstream ss;
			ss << "Milk - " << fpsCounter << " FPS";
			window->setTitle(ss.str());

			fpsCounter = 0;
		}
	}
}

void addPlayer()
{
	auto player =
	    Character::createCharacter(manager, "assets/images/mainCharacter.png");

	manager.addTag<TPlayer>(player);
	manager.getComponent<CLocation>(player) = {15 * 32, 15 * 32, 99};
}

int main(int argc, char** argv) {

	std::string a;
	sf::RenderWindow window_(sf::VideoMode(800, 600), "Milk");
	RenderSystem renderSystem_(manager, window_);
	MovementSystem movementSystem_(manager, overworld);
	InputSystem inputSystem_(manager);
	AnimationSystem animationSystem_(manager);
	
	window = &window_;
	renderSystem = &renderSystem_;
	movementSystem = &movementSystem_;
	inputSystem = &inputSystem_;
	animationSystem = &animationSystem_;

	overworld.loadMap("assets/maps/lake.tmx", TileLocation{0, 0});
	addPlayer();
	
	quit = false;

	loop();
	window_.close();

	return EXIT_SUCCESS;

}
