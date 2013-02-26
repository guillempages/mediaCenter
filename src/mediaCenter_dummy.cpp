#include <iostream>

#include <unistd.h>

#include "defines.h"
#include "utils.h"

int main(int argc, char * argv[]) {

    DBG(
       std::cout << argv[0];
       for (int i=1; i < argc ; ++i) {
           std::cout << " " << argv[i];
       }
       std::cout << std::endl;)

    while (1) {
        sleep(1);
    }
}
