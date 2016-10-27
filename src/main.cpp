#include <core/manager.hpp>
#include <input/input.hpp>
#include <render/animation.hpp>
#include <render/render.hpp>
#include <world/movement.hpp>
#include <world/overworld.hpp>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <cstdlib>

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

void addPlayer()
{
	auto player = manager.createEntity();

	manager.addTag<TPlayer>(player);
	manager.addTag<TAnimated>(player);
	manager.addComponent(player, CLocation{15 * 32, 15 * 32, 0});
	manager.addComponent(player, CDesiredMovement{Direction::NONE});
	manager.addComponent(player, CMovement{Direction::NONE, TileLocation{0, 0}});

	// TODO Move to animations loading function
	// TODO Clean this up, add all animations
	sf::Texture* spriteSheet = new sf::Texture;
	spriteSheet->loadFromFile("assets/images/mainCharacter.png");

	auto animation = new AnimatedSprite(*spriteSheet, sf::Vector2i(64, 64));
	manager.addComponent(player, CDrawable{animation});

	std::array<std::string, 4> animName{
		std::string("up"), std::string("left"), 
		std::string("down"), std::string("right")
	};	
	for(auto walkRow = 8; walkRow < 8 + 4; walkRow++)
	{
		std::vector<std::size_t> frames;
		for(auto i = 0u; i < 9; i++)
			frames.push_back(i + walkRow * 13);

		animation->addAnimation("walk_" + animName[walkRow - 8], frames);
	}

	animation->setPosition(-16.f, -32.f);
	//animation->setScale(sf::Vector2f(0.5f, 0.5f));
}

int main(int argc, char** argv) {

	std::string a;
	sf::RenderWindow window_(sf::VideoMode(800, 600), "Test");
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
