#include <core/manager.hpp>
#include <input/input.hpp>
#include <render/animation.hpp>
#include <render/character.hpp>
#include <render/dialoguebox.hpp>
#include <render/render.hpp>
#include <script/dialogue.hpp>
#include <script/script.hpp>
#include <world/movement.hpp>
#include <world/overworld.hpp>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <cstdlib>
#include <memory>
#include <sstream>
#include <string>

Manager manager;
Overworld overworld(manager, 30);

std::unique_ptr<sf::RenderWindow> window;

std::unique_ptr<InputSystem> inputSystem;
std::unique_ptr<MovementSystem> movementSystem;
std::unique_ptr<RenderSystem> renderSystem;
std::unique_ptr<AnimationSystem> animationSystem;
std::unique_ptr<ScriptSystem> scriptSystem;

bool quit;

std::unique_ptr<sf::Music> music;
std::string lastPlaying;

void lastMinuteAudioSystemDo()
{
	TileLocation playerLocation;

	manager.forEntitiesHaving<TPlayer>(
	[&playerLocation](EntityIndex player) 
	{
		playerLocation = TileLocation::fromLocation(
		    manager.getComponent<CLocation>(player));
	});

	std::string play = overworld.getMusic(playerLocation);
	if(play == "")
		play = "assets/music/default.flac";

	if(lastPlaying != play)
	{
		music->openFromFile(play);
		music->setLoop(true);

		music->play();

		lastPlaying = play;
	}
}

void update(sf::Time delta) 
{
	inputSystem->clearKeyPresses();

	sf::Event event;
	while(window->pollEvent(event))
	{
		if(event.type == sf::Event::Closed)
			quit = true;
		else if(event.type == sf::Event::KeyPressed)
			inputSystem->setKeyPressed(event.key.code);
	}

	inputSystem->update(delta);
	scriptSystem->update(delta);
	movementSystem->update(delta);
	animationSystem->update(delta);
	renderSystem->update(delta);

	lastMinuteAudioSystemDo();

	// TODO Move event cleanup elsewhere
	manager.forEntitiesHaving<TEvent>([](EntityIndex eI)
	{
		manager.deleteEntity(eI);
	});
}

void render(sf::Time delta)
{
	window->clear(sf::Color::Black);

	renderSystem->render(delta);

	window->display();
}

unsigned int fpsCounter;
sf::Time fpsTimer;

void loop() 
{
	sf::Clock clock;
	sf::Time accumulator;

	const auto max_frame_time = sf::milliseconds(100);
	const auto update_time_step = sf::milliseconds(30);

	while(!quit) 
	{
		auto elapsed = clock.restart();

		if(elapsed > max_frame_time)
			elapsed = max_frame_time;

		accumulator += elapsed;
		while(accumulator >= update_time_step) {
			update(update_time_step);

			accumulator -= update_time_step;
		}

		render(accumulator);

		// Display fps
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
	manager.addComponent(player, CPlayer{false});

	//manager.getComponent<CLocation>(player) = {29 * 32, 139 * 32, 99};
	//manager.getComponent<CLocation>(player) = {29 * 32, 19 * 32, 99};
	manager.getComponent<CLocation>(player) = {7 * 32, 19 * 32, 99};

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

	auto& scripts = manager.addComponent(follower, CScripts{});
	scripts["Name"] = "Orc";
	scripts["OnCombat"] = R"(
			dialog("<b>Orc</b> : Yes, boss!", fun()
			{
				walk_in_enemy_melee_range(fun()
				{
					damage(combat_enemy, 25);
					resume_combat();
				});
			});
	)";
	scripts["OnDefeat"] = R"(
			dialog("<b>Orc</b> : I am finished!", fun()
			{
				play(me, "stand_down", 200, fun()
				{
					resume_combat();
				});
			});
	)";

	return follower;
}

int main(int argc, char** argv) 
{
	window = std::make_unique<sf::RenderWindow>(
	    sf::VideoMode(480, 480), "Milk",
	    sf::Style::Titlebar | sf::Style::Close);

	renderSystem = std::make_unique<RenderSystem>(manager, *window);
	movementSystem = std::make_unique<MovementSystem>(manager, overworld);
	inputSystem = std::make_unique<InputSystem>(manager);
	animationSystem = std::make_unique<AnimationSystem>(manager);
	scriptSystem = std::make_unique<ScriptSystem>(manager, overworld);

	music = std::make_unique<sf::Music>();
	
	for(auto i = 0; i < 7; i++)
		for(auto j = 0; j < 5; j++)
		{
			TileLocation location{30 * i, 30 * j};

			auto index = std::to_string(1 + i + j * 7);
			if(index.size() == 1)
				index = "0" + index;
			
			overworld.loadMap("assets/maps/t" + index + ".tmx", location);
		}
	
	EntityIndex player = addPlayer();

	EntityIndex following = player;
	for(auto i = 0u; i < 0; i++)
	{
		following = addFollower(following);
	}

	quit = false;

	loop();
	window->close();

	return EXIT_SUCCESS;
}
