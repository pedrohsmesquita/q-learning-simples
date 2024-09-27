#include <iostream>
#include <random>
#include <algorithm>
#include <thread>
#include <chrono>
#include <string>

constexpr int G_SIZE            { 3 }; 
constexpr int NUM_ACTIONS       { 4 }; 
constexpr int EPSILON_GREEDY    { 100 };
constexpr int TIME_MS           { 1 };
constexpr double ALPHA           { 0.1 };
constexpr double GAMMA           { 0.9 };
constexpr double EPSILON         { 0.1 };

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution distrib(0,100);

struct State {
    double up;
    double down;
    double left;
    double right;
};

struct Entity {
    int state;
    int destination;
};

enum Actions {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

using StateGetVal = double(*)(const State&);
using StateSetVal = void(*)(State&, double);

State states[G_SIZE * G_SIZE] {};

int grid[G_SIZE * G_SIZE] = {0};

double getStateUp(const State& obj);
double getStateDown(const State& obj);
double getStateLeft(const State& obj);
double getStateRight(const State& obj);
void setStateUp(State& obj, double val);
void setStateDown(State& obj, double val);
void setStateLeft(State& obj, double val);
void setStateRight(State& obj, double val);
int getRandomNumber();
void setDestination(Entity& entity);
void clearConsole();
void updateGrid(const Entity& entity);
void printGrid();
Actions chooseAction(const State& state);
int actionOnGrid(const Actions action, const Entity &entity, int& pos_r, int& pos_c);
bool isWithinLimits(int pos_r, int pos_c);
int reward(int pos);
void updateQTable(const Entity& entity, int new_pos, int rwrd, Actions action);
void qLearning(Entity& entity);

StateGetVal getOperations[] = {
    getStateUp,
    getStateDown,
    getStateLeft,
    getStateRight
};

StateSetVal setOperations[] = {
    setStateUp,
    setStateDown,
    setStateLeft,
    setStateRight
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "usage: {executable} <num_of_iterations>" << std::endl;
        return 1;
    }
    int iterations {};
    try {
        iterations = std::stoi(argv[argc - 1]);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: invalid argument" << std::endl;
        return 1;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: number out of range" << std::endl;
        return 1;
    }
    
    Entity entity;

    setDestination(entity);

    for (int i = 0; i < iterations; i++) {;
        entity.state = entity.destination;
        while (entity.state == entity.destination) {
            entity.state = getRandomNumber() % (G_SIZE * G_SIZE);
        }
        clearConsole();
        updateGrid(entity);
        printGrid();
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_MS));
        qLearning(entity);
    }
    clearConsole();

    for (int i = 0; i < G_SIZE * G_SIZE; i++) {
        std::cout << "Estado " << i << ": ";
        std::cout << getOperations[UP](states[i]) << " ";
        std::cout << getOperations[DOWN](states[i]) << " ";
        std::cout << getOperations[LEFT](states[i]) << " ";
        std::cout << getOperations[RIGHT](states[i]) << std::endl;
    }
    std::cout << "Destino: " << entity.destination << std::endl;
    return 0;
}

double getStateUp(const State& obj) {
    return obj.up;
}

double getStateDown(const State& obj) {
    return obj.down;
}

double getStateLeft(const State& obj) {
    return obj.left;
}

double getStateRight(const State& obj) {
    return obj.right;
}

void setStateUp(State& obj, double val) {
    obj.up = val;
}

void setStateDown(State& obj, double val) {
    obj.down = val;
}

void setStateLeft(State& obj, double val) {
    obj.left = val;
}

void setStateRight(State& obj, double val) {
    obj.right = val;
}

int getRandomNumber() {
    return distrib(gen);
}

void setDestination(Entity& entity) {
    int destination { getRandomNumber() % (G_SIZE*G_SIZE) };
    grid[destination] = 1;
    entity.destination = destination;
}

void clearConsole() {
    std::cout << "\033[2J\033[1;1H" << std::endl;
}

void updateGrid(const Entity& entity) {
    std::fill(std::begin(grid), std::end(grid), 0);

    for (int i { 0 }; i < G_SIZE * G_SIZE; i++) {
        if (i == entity.destination)
            grid[i] = 1;
        else if (i == entity.state)
            grid[i] = 4;
        else
            grid[i] = 0;
    }
}

void printGrid() {
    for (int i { 0 }; i < G_SIZE * G_SIZE; i++) {
        if (i % G_SIZE == 0)
            std::cout << "\n";
        std::cout << grid[i] << "  ";
    }
    std::cout << std::endl;
}

Actions chooseAction(const State& state) {
    if (((double) getRandomNumber() / EPSILON_GREEDY) < EPSILON)
        return (Actions) (getRandomNumber() % NUM_ACTIONS);

    Actions best_action { UP };
    for (Actions action { DOWN }; action <= RIGHT; action = static_cast<Actions>(action + 1)) {
        if (getOperations[action](state) > getOperations[best_action](state))
            best_action = action;
    }

    return best_action;
}

int actionOnGrid(const Actions action, const Entity &entity, int& pos_r, int& pos_c) {
    int position_r { entity.state / G_SIZE };
    int position_c { entity.state - position_r * G_SIZE };

    switch (action) {
    case UP:
        position_r--;
        break;
    case DOWN:
        position_r++;
        break;
    case LEFT:
        position_c--;
        break;
    case RIGHT:
        position_c++;
        break;
    }
    pos_r = position_r;
    pos_c = position_c;
    return position_r * G_SIZE + position_c;
}

bool isWithinLimits(int pos_r, int pos_c) {
    return pos_r >= 0 && pos_r < G_SIZE && pos_c >= 0 && pos_c < G_SIZE;
}

int reward(int pos) {
    if (grid[pos] == 1)
        return 100;
    return -1;
}

void updateQTable(const Entity& entity, int new_pos, int rwrd, Actions action) {
    double new_state_max_val { getOperations[UP](states[new_pos]) };
    for (Actions actn { DOWN }; actn <= RIGHT; actn = static_cast<Actions>(actn + 1)) {
        if (getOperations[actn](states[new_pos]) > new_state_max_val)
            new_state_max_val = getOperations[actn](states[new_pos]);
    }

    setOperations[action](states[entity.state], getOperations[action](states[entity.state]) + ALPHA * ((double) rwrd + GAMMA
                                                    * new_state_max_val
                                                    - getOperations[action](states[entity.state])));
}

void qLearning(Entity& entity) {
    while (grid[entity.state] != 1) {
        Actions action { chooseAction(states[entity.state]) };
        int pos_r, pos_c;

        int new_pos { actionOnGrid(action, entity, pos_r, pos_c) };

        if (!isWithinLimits(pos_r, pos_c))
            continue;
        int rwrd { reward(new_pos) };
        updateQTable(entity, new_pos, rwrd, action);

        entity.state = new_pos;
        clearConsole();
        updateGrid(entity);
        printGrid();
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_MS));
    }
}
