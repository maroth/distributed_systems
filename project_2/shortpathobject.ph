#ifndef _SHORTPATHOBJECT_H
#define _SHORTPATHOBJECT_H
#include <timer.h>
//--------------------------------------------------

class neighbours;  // Full declaration at the end of this file

parclass ShortPathObject
{ classuid(1001);

  public:
    // Constructors and destructors
    ShortPathObject() @{od.url("localhost");};
    ShortPathObject(POPString m) @{od.url(m);};
    ~ShortPathObject();

    // Methods to construct the network
    async seq void initComputation(int dist, int NbN);
    sync  seq void setNeighbours(int NeighNo, ShortPathObject& node, int d);

    // Utility methods
    sync  conc ShortPathObject& getNeighbour(int NeighNo);
    async seq void setMyId(int i);
    sync  conc int getMyId();

    // Methods to run the computation
    async seq void startComputation();
    async seq void stop(int id);
    async seq void dist(int id, ShortPathObject& node, int val);
    async seq void ack(int id);

    // Methods fo getting statistics and results
    sync conc void getStatistic(int &nbDi, int &nbAc, int &nbSto);
    sync conc double getTime();
    sync conc int  getPath();

  private:   
    void initData();
    unsigned long my_ftime();
    
    int myId,                      // My Id (1..N)
        nbNeighbours,              // Number of neighbours
        nbRequests,                // Total requests (DIST) sent
        myPath;                    // My current value for the shortest path the
                                   // the initiator
    ShortPathObject* lastPred;     // last sender of a DIST message

    bool initiator,                // Am I the initiator of the computation ?
         terminated;               // Do I already received a stop message
    neighbours* myNeighbours;      // List of my neighbours

    // for statistical purpose
    int nbDist, nbAck, nbStop;
    POPSynchronizer endComputation;
    Timer timer;
    double computeTime;
};

class neighbours // contents information on a neighbour
  {
    public:
      int id;                      // Id of the neighbour
      ShortPathObject* theNode;    // Ref. to the neighbour (node)
      int dist;                    // Weight of the link to the neighbour
  };
	
#endif
