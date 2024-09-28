#include <iostream>
#include <random>
#include <algorithm>
#include <memory>
#include <thread>
#include <chrono>
#include <string>
#include <iomanip>

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

struct State;

struct Action {
    double val;
    Actions move;
    State& node_s;
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

    Action(State& state_ref, Actions mv) : val { 0 }, move { mv }, node_s { state_ref }, next { nullptr } {}
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

void printState();
void createAction(std::unique_ptr<Action>& head, State& state_ref, Actions move);
void setStateActions();
int getRandomNumber();
void setDestination(Entity& entity);
void clearConsole();
void updateGrid(const Entity& entity);
void printGrid();
Action *chooseAction(const State& state);
int actionOnGrid(const Actions action, int& pos_r, int& pos_c);
bool isWithinLimits(int pos_r, int pos_c);
void updateQTable(int new_pos, Actions action);
void qLearning(Entity& entity);

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "usage: {executable} <num_of_iterations>" << std::endl;
        return 1;
    }
    int iterations {1};
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
    setStateActions();

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
    printState();
    std::cout << "Destino: " << entity.destination << std::endl;

    return 0;
}

void printState() {
    std::cout << "Estado\tUP\tDOWN\tLEFT\tRIGHT\n";
    for (int i { 0 }; i < G_SIZE * G_SIZE; i++) {
        std::cout << i << "\t";
        Action *move = states[i].head.get();
        for (Actions actn { }; actn <= RIGHT; actn = static_cast<Actions>(actn + 1)) {
            if (move && actn == move->getMove()) {
                std::cout << std::fixed << std::setprecision(4) << move->getVal() << "\t";
                move = move->next.get();
            } else {
                std::cout << std::fixed << std::setprecision(4) << 0.0 << "\t";
            }
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
}

void createAction(std::unique_ptr<Action>& head, State& state_ref, Actions move) {
    auto new_node = std::make_unique<Action>(state_ref, move);
    if(!head) {
        head = std::move(new_node);
        return ;
    }
    Action *current = head.get();
    while (current->next)
        current = current->next.get();
    current->next = std::move(new_node);
}

void setStateActions() {
    for (int r { }; r < G_SIZE; r++) {
        for (int c { }; c < G_SIZE; c++) {
            int pos = r * G_SIZE + c;
            if (r - 1 >= 0) {
                createAction(states[pos].head, states[(r - 1) * G_SIZE + c], UP);
            }
            if (r + 1 < G_SIZE) {
                createAction(states[pos].head, states[(r + 1) * G_SIZE + c], DOWN);
            }
            if (c - 1 >= 0) {
                createAction(states[pos].head, states[(c - 1) * G_SIZE + c], LEFT);
            }
            if (c + 1 < G_SIZE) {
                createAction(states[pos].head, states[(c + 1) * G_SIZE + c], RIGHT);
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

Action *chooseAction(const State& state) {
    static Action *raw_ptrs[NUM_ACTIONS];
    int qtd {0};
    Action *current { state.head.get() };
    Action *max { current };
    raw_ptrs[qtd++] = current;

    current = current->next.get();
    while (current) {
        raw_ptrs[qtd++] = current;
        if (current->getVal() > max->getVal())
            max = current;
        current = current->next.get();
    }

    current = raw_ptrs[getRandomNumber() % qtd];
    std::fill(std::begin(raw_ptrs), std::end(raw_ptrs), nullptr);

    if (((double) getRandomNumber() / EPSILON_GREEDY) < EPSILON)
        return current;

    return max;
}

int actionOnGrid(const Actions action, int& pos_r, int& pos_c) {
    switch (action) {
    case UP:
        pos_r--;
        break;
    case DOWN:
        pos_r++;
        break;
    case LEFT:
        pos_c--;
        break;
    case RIGHT:
        pos_c++;
        break;
    }

    return pos_r * G_SIZE + pos_c;
}

void updateQTable(int new_pos, Action *actual_action) {
    Action *current { states[new_pos].head.get() };
    Action *max_action_new_state { current };
    current = current->next.get();

    while (current) {
        if (current->getVal() > max_action_new_state->getVal())
            max_action_new_state = current;
        current = current->next.get();
    }

    actual_action->setVal(ALPHA * (states[new_pos].getReward() + GAMMA * max_action_new_state->getVal() - actual_action->getVal()));
}

void qLearning(Entity& entity) {
    while (entity.state != entity.destination) {
        Action *state_action { chooseAction(states[entity.state]) };
        int pos_r { entity.state / G_SIZE };
        int pos_c { entity.state - pos_r * G_SIZE };

        int new_pos { actionOnGrid(state_action->getMove(), pos_r, pos_c) };

        updateQTable(new_pos, state_action);

        entity.state = new_pos;
        clearConsole();
        updateGrid(entity);
        printGrid();
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_MS));
    }
}
