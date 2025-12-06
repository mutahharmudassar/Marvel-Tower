#include <SFML/Graphics.hpp>
#include <iostream>

int main() {
    std::cout << "Starting test..." << std::endl;
    
    sf::RenderWindow window(sf::VideoMode(800, 600), "Test Window");
    std::cout << "Window created!" << std::endl;
    
    while (window.isOpen()) {
        sf::Event event;
        while (window. pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        
        window. clear(sf::Color::Blue);
        window. display();
    }
    
    std::cout << "Test complete!" << std::endl;
    return 0;
}