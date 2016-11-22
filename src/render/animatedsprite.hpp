#pragma once 

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <cassert>
#include <string>
#include <unordered_map>

class AnimatedSprite : public sf::Drawable, public sf::Transformable
{
public:
	AnimatedSprite(sf::Texture& spriteSheet, sf::Vector2i frameSize)
		: spriteSheet(spriteSheet), frameSize(frameSize),
		  sprite(spriteSheet, sf::IntRect(sf::Vector2i(0, 0), frameSize)),
		  timePerFrame(sf::milliseconds(1000))
	{
		addAnimation("default", std::vector<std::size_t>{0});
		playAnimation("default", true);
		stopAnimation();
	}

private:
	sf::Sprite sprite;
	sf::Texture& spriteSheet;

	sf::Vector2i frameSize;

public:
	void addAnimation(const std::string& name, 
	                  const std::vector<std::size_t>& frames)
	{
		animationFrames[name] = frames;
	}

	void setDefaultAnimation(const std::vector<std::size_t>& frames)
	{
		animationFrames[std::string("default")] = frames;
	}

	void playAnimation(const std::string& name, bool loop = false)
	{
		animationFrameIndex = 0;
		accumulator = sf::milliseconds(0);

		currentAnimation = name;
		looping = loop;

		playing = true;
	}

	bool isPlaying() const
	{
		return playing;
	}

	std::string getCurrentAnimation() const
	{
		return currentAnimation;
	}

	void stopAnimation()
	{
		playing = false;
	}

	void setPlaySpeed(sf::Time perFrame)
	{
		timePerFrame = perFrame;
	}

	void update(sf::Time delta)
	{
		if(!playing)
			return;

		auto& animation = animationFrames[currentAnimation];

		accumulator += delta;
		while(accumulator >= timePerFrame)
		{
			accumulator -= timePerFrame;

			animationFrameIndex += 1;
			if(looping)
				animationFrameIndex %= animation.size();
			else if(animationFrameIndex == animation.size())
			{
				stopAnimation();
				return;
			}
		}

		auto animationFrame = animation[animationFrameIndex];
		sprite.setTextureRect(frameToTextureRect(animationFrame));
	}

private:
	sf::IntRect frameToTextureRect(std::size_t frame) const
	{
		auto framesWide = spriteSheet.getSize().x / frameSize.x;

		auto x = frame % framesWide;
		auto y = frame / framesWide;

		auto framePosition = sf::Vector2i(x * frameSize.x, y * frameSize.y); 

		return sf::IntRect(framePosition, frameSize);
	}

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		states.transform *= getTransform();
		states.texture = &spriteSheet;

		target.draw(sprite, states); 
	}

private:
	std::unordered_map<std::string, std::vector<std::size_t>> animationFrames;

	std::string currentAnimation;
	std::size_t animationFrameIndex;

	sf::Time accumulator;
	sf::Time timePerFrame;

	bool looping;
	bool playing;
};
