#include <chrono>
#include <thread>
#include <ctime>
#include <cstdlib>


// Create delay between 0 and maxDelay milliseconds
void CreateRandomDelay(int maxDelay);

// Decide whether to throw an error, with 1/chanceDenom chance
bool IsCreateRandomError(int chanceDenom);
