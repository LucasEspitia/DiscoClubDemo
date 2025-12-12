#include <iostream>
#include <glm/glm.hpp>
#include "window/ClubbingWindow.h"
//#include <direct.h>



int main() {
    // Create new window
    auto window = ClubbingWindow{};

    // Main execution loop
    while (window.pollEvents()) {}

    return EXIT_SUCCESS;

}
