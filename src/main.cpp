#include <core/manager.hpp>
#include <input/input.hpp>
#include <render/animation.hpp>
#include <render/character.hpp>
#include <render/dialoguebox.hpp>
#include <render/render.hpp>
#include <script/dialogue.hpp>
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

EntityIndex addPlayer()
{
	auto player =
	    Character::createCharacter(manager, "assets/images/mainCharacter.png");

	manager.addTag<TPlayer>(player);
	//manager.getComponent<CLocation>(player) = {7 * 32, 19 * 32, 99};
	manager.getComponent<CLocation>(player) = { 107 * 32, 119 * 32, 99 };

	return player;
}

EntityIndex addFollower(EntityIndex entityAhead)
{
	auto follower = 
		Character::createCharacter(manager, "assets/images/orc1Enemy.png");

	auto aheadLocation = manager.getComponent<CLocation>(entityAhead);

	manager.addTag<TFollower>(follower);
	manager.getComponent<CLocation>(follower) = {
	    aheadLocation.x, aheadLocation.y, 99};
	manager.addComponent(follower, CFollowOrder{ entityAhead });

	return follower;
}

void addDialogueBox()
{
	Dialogue::createDialogue(
	    manager,
	    "<b>You: </b>What am I doing with my life?<br/>"
		"<b>Orcs: </b><i>Here comes dat orc!</i><br/>"
		"<b>oh shit warlock: </b>"
		"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Morbi magna enim, semper a ex id, laoreet rutrum ex. Morbi fermentum ipsum et sapien luctus egestas. Phasellus vitae purus non eros mollis semper. Etiam laoreet euismod vulputate.");
}

int main(int argc, char** argv) {

	std::string a;
	sf::RenderWindow window_(
	    sf::VideoMode(480, 480), "Milk",
	    sf::Style::Titlebar | sf::Style::Close);
	RenderSystem renderSystem_(manager, window_);
	MovementSystem movementSystem_(manager, overworld);
	InputSystem inputSystem_(manager);
	AnimationSystem animationSystem_(manager);
	
	window = &window_;
	renderSystem = &renderSystem_;
	movementSystem = &movementSystem_;
	inputSystem = &inputSystem_;
	animationSystem = &animationSystem_;

	for(auto i = 0u; i < 7; i++)
		for(auto j = 0u; j < 5; j++)
		{
			TileLocation location{ 30 * i, 30 * j };

			auto index = std::to_string(1 + i + j * 7);
			if(index.size() == 1)
				index = "0" + index;
			
			overworld.loadMap("assets/maps/t" + index + ".tmx", location);
		}
	
	EntityIndex player =  addPlayer();

	EntityIndex following = player;
	for(auto i = 0u; i < 32; i++)
	{
		following = addFollower(following);
	}

	addDialogueBox();
	
	quit = false;

	loop();
	window_.close();

	return EXIT_SUCCESS;

}
