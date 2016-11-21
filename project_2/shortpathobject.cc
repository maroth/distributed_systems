#include <stdlib.h>
#include <iostream>
#include <sys/timeb.h>  // ftime
#include "shortpathobject.ph"

//-----------------------------------
// Private methods
//-----------------------------------
// return local time (LT) in msec
unsigned long  ShortPathObject::my_ftime()
{
  struct timeb t;
  ftime(&t);     
  return (long)(t.time)*1000 + (long)(t.millitm);
}

//-----------------------------------
// Initialise node data
void ShortPathObject::initData()
{
  myNeighbours = NULL;
  nbNeighbours = 0;
  myPath = 65535;
  nbRequests = 0;
  lastPred = NULL;
  initiator = false;
  terminated = false;
  myId = -1;
  nbDist = nbAck = nbStop = 0;
  endComputation.lock();
}

//-----------------------------------
// Constructors and destructors

ShortPathObject::ShortPathObject()
{//printf("Creating node on %s\n", POPGetHost());
  initData();
}

//-----------------------------------
ShortPathObject::ShortPathObject(POPString m)
{//printf("Creating node on %s\n", POPGetHost());
  initData();
}

//-----------------------------------
ShortPathObject::~ShortPathObject()
{//printf("Destroying node %2d\n", myId);
  if (myNeighbours != NULL) free(myNeighbours); myNeighbours=NULL;
  if (lastPred!=NULL) delete lastPred; lastPred = NULL;
}

//-----------------------------------
//-----------------------------------
// Methods to construct the network

void ShortPathObject::initComputation(int dist, int NbN)
{
  myPath = dist;     // Set big enough value for myPath (should be infinite)
  myNeighbours = (neighbours*)malloc(NbN * sizeof(neighbours));

  for (int i=0; i<NbN; i++)
  {
    myNeighbours[i].id = -1;         // ID of the neighbour
    myNeighbours[i].theNode = NULL;  // ref. to the neighbour
    myNeighbours[i].dist = 1;        // Weight of the link to the neig.
  }
  nbNeighbours = NbN;
}

//-----------------------------------
void ShortPathObject::setNeighbours(int NeighNo, ShortPathObject& node, int d)
{
  myNeighbours[NeighNo].theNode = new ShortPathObject(node);
  myNeighbours[NeighNo].dist = d;
  myNeighbours[NeighNo].id = myNeighbours[NeighNo].theNode->getMyId();
}

//-----------------------------------
// Utility methods

ShortPathObject& ShortPathObject::getNeighbour(int NeighNo)
{ return *(myNeighbours[NeighNo].theNode); }

void ShortPathObject::setMyId(int i) { myId = i; }

int ShortPathObject::getMyId() { return myId; }

//-----------------------------------
// Methods to run the computation

void ShortPathObject::startComputation()
{
  initiator = true;            // I am the initiator
  myPath = 0;                  // My distance to me is zero 
  nbRequests = nbNeighbours;   // I send a request to all my neigh.
  timer.Start();               // I start the chrono

  // I send the requests (<dist> messages) to all my Neigbours
  for (int i=0; i<nbNeighbours; i++)
    myNeighbours[i].theNode->dist(myId, *this, myNeighbours[i].dist);
}

//-----------------------------------
void ShortPathObject::dist(int id, ShortPathObject& node, int val)
{//printf("%12lu:: %2d recieved DIST(%2d) from %2d\n", my_ftime(), myId, val, id);
  nbDist++;   // Statistic

  if (val<myPath)
  { // I received a better distance
    myPath = val;   // I update my distance

    if (nbNeighbours==0)
      node.ack(myId); // I am a terminal node
    else              // I am not a terminal node
    {
      if ((nbRequests>0) && (lastPred!=NULL) )
      {
        lastPred->ack(myId);
        delete lastPred;
      }
      lastPred = new ShortPathObject(node); // I update the predecessor
      for (int i=0; i<nbNeighbours; i++)
      {
        // I propagate my new distance to my neighbours
        myNeighbours[i].theNode->
                        dist(myId, *this, myPath + myNeighbours[i].dist);
        nbRequests++;
      } 
    } 
  }
  else node.ack(myId); // The received distance is not interesting
 }

//-----------------------------------
void ShortPathObject::ack(int id)
{//printf( "%12lu:: %2d recieved ACK(%3d) from %2d\n", my_ftime(), myId, nbRequests,id);

  nbAck++;       // Statistic

  nbRequests--;  
  if (nbRequests==0)
  {
    if (initiator)
    {
      nbRequests = 0;
      computeTime = timer.Elapsed();
      for (int i=0; i<nbNeighbours; i++)
      {
        myNeighbours[i].theNode->stop(myPath);
        nbRequests++;
      }
    endComputation.unlock();
    }
    else  // I am not the initiator
      if (lastPred != NULL) lastPred->ack(myId); 
  }
}

//-----------------------------------
void ShortPathObject::stop(int id)
{//printf( "%12lu:: %2d recieved STOP(  ) from %2d\n", my_ftime(), myId, id);

  nbStop++;   // Statistic

  if (!terminated)
  {   
    endComputation.unlock();
    terminated = true;
    for (int i=0; i<nbNeighbours; i++)
      if (myNeighbours[i].id!=id) myNeighbours[i].theNode->stop(myId);
    for (int i=0; i<nbNeighbours; i++)
    {
      if (myNeighbours[i].theNode!=NULL) delete myNeighbours[i].theNode;
      myNeighbours[i].theNode = NULL;
    }    
  }
}

//-----------------------------------
void ShortPathObject::getStatistic(int &nbDi, int &nbAc, int &nbSto)
{
  endComputation.lock();
  nbDi = nbDist;
  nbAc = nbAck;
  nbSto = nbStop;
  endComputation.unlock();
}

//-----------------------------------
double ShortPathObject::getTime()
{
  double cT;
  endComputation.lock();
  cT = computeTime;
  endComputation.unlock();
  return cT;
}

//-----------------------------------
int ShortPathObject::getPath()
{
  int p;
  endComputation.lock();
  p=myPath;
  endComputation.unlock();
  return p;
}

@pack(ShortPathObject);
