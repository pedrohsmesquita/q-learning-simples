#include <iostream>
#include <random>
#include <algorithm>
#include <memory>
#include <thread>
#include <chrono>
#include <string>

constexpr int G_SIZE            { 3 }; 
constexpr int REWARD_DEST       { 10 };
constexpr int NUM_ACTIONS       { 4 }; 
constexpr int EPSILON_GREEDY    { 100 };
constexpr int TIME_MS           { 1 };
constexpr double ALPHA           { 0.1 };
constexpr double GAMMA           { 0.9 };
constexpr double EPSILON         { 0.1 };

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution distrib(0,100);

enum Actions {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

struct Action {
    double val;
    Actions move;
    State *node_s;
    std::unique_ptr<Action> next;

    double getVal() {
        return val;
    }

    void setVal(double value) {
        val = value;
    }

    Actions getMove() {
        return move;
    }

    void setMove(Actions mv) {
        move = mv;
    }

    Action(State *state_ptr, Actions mv) : val { }, move { mv }, node_s { state_ptr }, next { nullptr } {}
};

struct State {
    int reward;
    std::unique_ptr<Action> head;

    int getReward() {
        return reward;
    }

    void setReward(int rwrd) {
        reward = rwrd;
    }

    State() : reward { }, head { nullptr } {}
};

struct Entity {
    int state;
    int destination;
};

State states[G_SIZE * G_SIZE] {};

int grid[G_SIZE * G_SIZE] = {0};

void createAction(std::unique_ptr<Action>& head, State *state_ptr, Actions move);
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

    // for (int i = 0; i < G_SIZE * G_SIZE; i++) {
    //     std::cout << "Estado " << i << ": ";
    //     std::cout << getOperations[UP](states[i]) << " ";
    //     std::cout << getOperations[DOWN](states[i]) << " ";
    //     std::cout << getOperations[LEFT](states[i]) << " ";
    //     std::cout << getOperations[RIGHT](states[i]) << std::endl;
    // }
    std::cout << "Destino: " << entity.destination << std::endl;
    return 0;
}

void createAction(std::unique_ptr<Action>& head, State *state_ptr, Actions move) {
    auto new_node = std::make_unique<Action>(state_ptr, move);
    if(!head) {
        head = std::move(new_node);
        return ;
    }
    Action *current = head.get();
    while (current->next)
        current = current->next.get();
    current->next = std::move(new_node);
}

void setStateActions(Entity& entity) {
    for (int r { }; r < G_SIZE; r++) {
        for (int c { }; c < G_SIZE; c++) {
            int pos = r * G_SIZE + c;
            if (entity.destination == pos) {
                continue;
            }
            if (r - 1 >= 0) {
                createAction(states[pos].head, &states[(r - 1) * G_SIZE + c], UP);
            }
            if (r + 1 < G_SIZE) {
                createAction(states[pos].head, &states[(r + 1) * G_SIZE + c], DOWN);
            }
            if (c - 1 >= 0) {
                createAction(states[pos].head, &states[(c - 1) * G_SIZE + c], LEFT);
            }
            if (c + 1 < G_SIZE) {
                createAction(states[pos].head, &states[(c + 1) * G_SIZE + c], RIGHT);
            }
        }
    }
}

int getRandomNumber() {
    return distrib(gen);
}

void setDestination(Entity& entity) {
    int destination { getRandomNumber() % (G_SIZE*G_SIZE) };
    grid[destination] = 1;
    entity.destination = destination;
    states[destination].setReward(REWARD_DEST);
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
        return 10;
    return 0;
}

void updateQTable(const Entity& entity, int new_pos, int rwrd, Actions action) {
    double new_state_max_val { getOperations[UP](states[new_pos]) };
    for (Actions actn { DOWN }; actn <= RIGHT; actn = static_cast<Actions>(actn + 1)) {
        if (getOperations[actn](states[new_pos]) > new_state_max_val)
            new_state_max_val = getOperations[actn](states[new_pos]);
    }

    setOperations[action](states[entity.state],     ALPHA * ((double) rwrd + GAMMA
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
