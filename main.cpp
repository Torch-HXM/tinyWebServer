#include "log.h"
#include "reactor.h"

int main(){
    auto reactor_ptr = reactor::Reactor::getInstance();
    reactor_ptr->play();
}