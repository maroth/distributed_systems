#include "helloworld.ph"
#include <unistd.h>

HelloWorld::HelloWorld()
{
  id = -1;
  printf("HelloWorld object created on %s\n", POPGetHost());
}
 
void HelloWorld::setID(int i)
{
  sleep(1);
  id = i;
  printf("Hello World from object : %d\n", id);
}

int HelloWorld::getID()
{
  return id;
}
 
@pack(HelloWorld);

