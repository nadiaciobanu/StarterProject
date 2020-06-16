#include "food_utils.h"


void CreateRandomDelay(int maxDelay) {
    srand(time(0));
    int delay = rand() % maxDelay;

    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
}


bool IsCreateRandomError(int chanceDenom) {
    srand(time(0));
    int random_number = rand() % chanceDenom;
    if (random_number == 0) {
        return true;
    }
    return false;
}
