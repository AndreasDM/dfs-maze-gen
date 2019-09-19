#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <utility>
#include <cstdlib>
#include <thread>
#include <chrono>
using namespace std::chrono_literals;

struct Node {
  sf::RectangleShape  shape;
  std::vector<Node*>  adj;
  std::pair<int, int> pos;
  bool                obstacle = true;
  bool                visited  = false;

  Node(int posX, int posY, int x, int y, int w, int h, sf::Color color)
    : shape{ { float(w), float(h) } }
    , adj{}
    , pos{ posX, posY } {
    shape.setPosition(x, y);
    shape.setFillColor(color);
  }
};

void findAdj(std::vector<Node>& grid, int const& bW, int const& bH) {
  for (int i = 0; i != grid.size(); ++i) {
    int x{ i % bW }, y{ i / bH };
    if (!(x - 2 < 0))   grid[i].adj.push_back(&grid[i - 2]);
    if (!(x + 2 >= bW)) grid[i].adj.push_back(&grid[i + 2]);
    if (!(y - 2 < 0))   grid[i].adj.push_back(&grid[i - bW*2]);
    if (!(y + 2 >= bH)) grid[i].adj.push_back(&grid[i + bW*2]);
  }
}

inline int pairToIndex(std::pair<int, int>& p, int bh) { return p.first + p.second * bh; }

int main() {
  const int        boardW = 1030;
  const int        boardH = 1030;
  sf::RenderWindow window{ { boardW, boardH }, "Game" };

  window.setPosition({ 0, 0 });

  int const  w     = 30;
  int const  bW    = boardW / w;
  int const  bH    = boardH / w;
  auto const delay = 30ms;

  std::vector<Node> grid;
  for (int i{}, z{}, y{ 0 }; i != bH; ++i, y += w)
    for (int j{}, x{ 0 }; j != bW; ++j, x += w, ++z)
      grid.emplace_back(Node(z % bW, z / bH, x, y, w, w, sf::Color::Black));

  findAdj(grid, bW, bH);

  auto* currentCell = &grid.at(1 + bW);
  currentCell->shape.setFillColor(sf::Color::White);

  std::deque<Node*> stack;
  std::deque<Node*> walls;
  currentCell->visited = true;
  stack.push_back(currentCell);

  auto reset = [&stack, &walls, &grid, &currentCell]() {
    stack.clear(); walls.clear();
    for (auto & i : grid) {
      i.shape.setFillColor(sf::Color::Black);
      i.obstacle = true;
      i.visited  = false;
    }
    currentCell = &grid.at(1 + bW);
    currentCell->shape.setFillColor(sf::Color::White);
    currentCell->visited = true;
    stack.push_back(currentCell);
  };

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::KeyPressed) {
        switch (event.key.code) {
          case sf::Keyboard::Escape : window.close(); break;
          case sf::Keyboard::Space  : reset();        break;
          default                   :                 break;
        }
      }
    }

    window.clear(sf::Color::Black);

    if (!stack.empty()) {
      if (std::any_of(std::begin(currentCell->adj), std::end(currentCell->adj), [](auto& i) { return !i->visited && i->obstacle; })) {
        std::vector<Node*> cpy{};
        std::copy_if(std::begin(currentCell->adj), std::end(currentCell->adj), std::back_inserter(cpy), [](auto& i) { return !i->visited; });
        auto* w = cpy[std::rand() / (RAND_MAX/(cpy.size()))];

        // find wall between current and neighbor
        std::pair<int, int> tmp;
        if (currentCell->pos.first == w->pos.first) {
          int maxComp = std::max(currentCell->pos.second, w->pos.second);
          tmp         = { w->pos.first, maxComp - 1 };
        } else {
          int maxComp = std::max(currentCell->pos.first, w->pos.first);
          tmp         = { maxComp - 1, w->pos.second };
        }

        grid[pairToIndex(tmp, bH)].shape.setFillColor(sf::Color::White);
        for (auto const& i : grid)
          window.draw(i.shape);
        window.display();
        std::this_thread::sleep_for(delay);
        walls.push_back(&grid[pairToIndex(tmp, bH)]);

        w->obstacle = false;
        w->visited  = true;
        w->shape.setFillColor(sf::Color::White);
        stack.push_back(w);
        currentCell = w;

        for (auto const& i : grid)
          window.draw(i.shape);
        window.display();
        std::this_thread::sleep_for(delay);
      }
      else {
        currentCell = stack.back();
        currentCell->shape.setFillColor(sf::Color::Blue);
        stack.pop_back();

        for (auto const& i : grid)
          window.draw(i.shape);
        window.display();
        std::this_thread::sleep_for(delay);

        if (Node* lastwall{nullptr}; !walls.empty()) {
          lastwall = walls.back();
          lastwall->shape.setFillColor(sf::Color::Blue);
          walls.pop_back();

          for (auto const& i : grid)
            window.draw(i.shape);
          window.display();
        }
        std::this_thread::sleep_for(delay);
      }
    }
    else {
      for (auto const& i : grid)
        window.draw(i.shape);
      window.display();
    }
  }
}
