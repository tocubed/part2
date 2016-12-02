#pragma once 

#include <pugixml.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <cassert>
#include <cctype>
#include <string>
#include <unordered_map>

class DialogueBox : public sf::Drawable, public sf::Transformable
{
public:
	DialogueBox(sf::Vector2i dimensions, sf::Texture& texture, sf::Font& font)
		: dimensions(dimensions), font(font)
	{
		visibleLines = (dimensions.y - 32) / 32;

		box.setTexture(texture);
	}

	void setText(const std::string& richText)
	{	
		clearText();
		
		currentLine = 0;
		visibleCharacters = 0;
		currentLinesDisplayed = false;

		pugi::xml_document richDoc;
		richDoc.load_string(
		    richText.c_str(), pugi::parse_default | pugi::parse_fragment);

		for(auto node: richDoc.children())
		{
			if(node.type() == pugi::xml_node_type::node_pcdata)
			{
				appendText(node.value());
			}
			else if(node.type() == pugi::xml_node_type::node_element)
			{
				unsigned int style = 0;

				if(node.name() == std::string("b") || node.attribute("b"))
					style |= sf::Text::Style::Bold;
				if(node.name() == std::string("i") || node.attribute("i"))
					style |= sf::Text::Style::Italic;
				if(node.name() == std::string("u") || node.attribute("u"))
					style |= sf::Text::Style::Underlined;
				if(node.name() == std::string("s") || node.attribute("s"))
					style |= sf::Text::Style::StrikeThrough;

				if(node.name() == std::string("br"))	
					appendNewline();
				else
					appendText(node.child_value(), style);
			}
		}
	}

	bool allDisplayed() const
	{
		return currentLinesDisplayed &&
		       ((currentLine + visibleLines) >= lines.size());
	}

	void displayMoreCharacters()
	{
		visibleCharacters++;
	}

	void displayMoreLines()
	{
		if(currentLinesDisplayed)
		{
			currentLine += visibleLines;
			visibleCharacters = 0;

			currentLinesDisplayed = false;
		}
	}

private:
	void clearText()
	{
		lines.clear();
		appendNewline();
	}

	void appendText(const std::string& str, unsigned int style = 0)
	{
		bool bold = (style & sf::Text::Style::Bold) != 0;

		int maxLineWidth = dimensions.x - 24;

		auto remaining = str;
		while(remaining.size() > 0)
		{
			auto i = calculateSplit(
			    remaining, maxLineWidth - currentLineWidth, 24, bold);

			auto piece = remaining.substr(0, i);
			remaining = remaining.substr(i);

			if(piece == "")
			{
				appendNewline();
				continue;
			}

			lines.back().emplace_back();
			auto& text = lines.back().back();

			text.setFont(font);
			text.setStyle(style);
			text.setCharacterSize(24);

			text.setString(piece);
			text.move(currentLineWidth, 0);

			currentLineWidth += text.getLocalBounds().width;
		}
	}

	void appendNewline()
	{
		lines.emplace_back();
		currentLineWidth = 24;
	}

	mutable std::vector<std::vector<sf::Text>> lines;
	unsigned int currentLineWidth;

private:
	std::size_t calculateSplit(const std::string& text, int maxWidth,
		                       unsigned int characterSize, bool bold)
	{
		auto i = 0;
		auto width = 0;

		while(i < text.size())
		{
			auto character = sf::Utf32::decodeAnsi(text[i]);

			width += font.getGlyph(character, characterSize, bold).advance;
			
			if(width > maxWidth)
				break;

			i++;
		}

		if(i != text.size())
			while(i > 0 && !std::isspace(text[i]))
				i--;

		return i;
	}

private:
	sf::Sprite box;
	sf::Vector2i dimensions;

	sf::Font& font;

	unsigned int currentLine;
	unsigned int visibleLines;
	unsigned int visibleCharacters;

	mutable bool currentLinesDisplayed;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		states.transform *= getTransform();

		target.draw(box, states);

		states.transform *= sf::Transform().translate(0, 16);

		auto characters = 0u;
		for(auto i = currentLine; i < currentLine + visibleLines; i++)
		{
			if(i >= lines.size())
				break;

			for(auto& text: lines[i])
			{
				characters += text.getString().getSize();

				if(characters > visibleCharacters)
				{
					auto str = text.getString();
					auto remaining = visibleCharacters + str.getSize() - characters;

					text.setString(str.substring(0, remaining));
					target.draw(text, states); 
					text.setString(str);

					return;
				}

				target.draw(text, states); 
			}

			states.transform *= sf::Transform().translate(0, 32);
		}

		currentLinesDisplayed = true;
	}
};
