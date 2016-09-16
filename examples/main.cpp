#include <SFML/Graphics.hpp>
#include <iostream>

#include "../src/LightSystem.hpp"

int main()
{
	sf::RenderWindow window(sf::VideoMode(800, 600), "Lumos", sf::Style::Close);
	window.setVerticalSyncEnabled(true);

	sf::View view = window.getDefaultView();

	sf::Texture penumbraTexture;
	penumbraTexture.loadFromFile("examples/penumbraTexture.png");
	penumbraTexture.setSmooth(true);

	sf::Shader unshadowShader;
	unshadowShader.loadFromFile("examples/unshadowShader.frag", sf::Shader::Fragment);

	sf::Shader lightOverShapeShader;
	lightOverShapeShader.loadFromFile("examples/lightOverShapeShader.frag", sf::Shader::Fragment);

	lum::LightSystem ls;
	ls.create(sf::FloatRect{ { 0.f, 0.f }, view.getSize() }, window.getSize(), penumbraTexture, unshadowShader, lightOverShapeShader);

	//----- Add a light

	sf::Texture directionLightTexture;
	directionLightTexture.loadFromFile("examples/directionLightTexture.png");
	directionLightTexture.setSmooth(true);

	sf::Texture pointLightTexture;
	pointLightTexture.loadFromFile("examples/pointLightTexture.png");
	pointLightTexture.setSmooth(true);

	sf::Texture spookyLightTexture;
	spookyLightTexture.loadFromFile("examples/spookyLightTexture.png");
	spookyLightTexture.setSmooth(true);

	//----- Add a square blocking light

	sf::RectangleShape blocker;
	blocker.setSize({ 200.f, 50.f });
	blocker.setPosition(500.f, 300.f);
	blocker.setFillColor(sf::Color::Red);

	auto lightBlocker = ls.createLightShape();
	lightBlocker->setPointCount(4u);
	lightBlocker->setPoint(0u, { 0.f, 0.f });
	lightBlocker->setPoint(1u, { 0.f, blocker.getSize().y });
	lightBlocker->setPoint(2u, blocker.getSize());
	lightBlocker->setPoint(3u, { blocker.getSize().x, 0.f });
	lightBlocker->setPosition(blocker.getPosition());

	auto sun = ls.createLightPointDirection();
	sun->setOrigin(sf::Vector2f(directionLightTexture.getSize().x * 0.5f, directionLightTexture.getSize().y * 0.5f));
	sun->setTexture(directionLightTexture);
	sun->setScale(sf::Vector2f(6.0f, 6.0f));
	sun->setColor(sf::Color(255, 230, 200));

	auto mlight = ls.createLightPointEmission();
	mlight->setOrigin(sf::Vector2f(pointLightTexture.getSize().x * 0.5f, pointLightTexture.getSize().y * 0.5f));
	mlight->setTexture(pointLightTexture);
	mlight->setScale(3.f, 3.f);
	mlight->setColor({ 200u, 200u, 250u });

	float angle = 22.5f;

	sf::Clock clock;
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
			{
				window.close();
			}
		}

		sf::Time dt = clock.restart();

		angle += 10.f * dt.asSeconds();
		if (angle > 157.5f)
		{
			angle = 22.5f;
		}
		sun->setCastAngle(angle);

		mlight->setPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window), view));

		sf::Vector2f mvt;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
			mvt.y--;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
			mvt.y++;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
			mvt.x--;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
			mvt.x++;
		view.move(mvt * dt.asSeconds() * 200.f);

		window.clear(sf::Color::White);
		window.setView(view);

		window.draw(blocker);

		ls.render(window);

		window.display();
	}

	return 0;
}