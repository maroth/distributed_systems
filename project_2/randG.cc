#include <stdio.h>
#include <stdlib.h>
#include "shortpathobject.ph"

#define ExcelFileName  "Excel.txt"
#define nbMaxMachines 200
#define MachinesList "machines.ip"


// Read  the file 'fileName' containing the list of machines,
//  fill the array 'machines' and returns the number of machines
int getAvailableMachines(char* fileName, POPString* machine[])

{
  int nbOfMachines = 0;
  FILE* f;
  if ( (f = fopen(fileName, "r"))!=NULL)
  {
    char* s;
    while (!feof(f))
    {
      s=(char*)malloc(100);
      fscanf(f, "%s", s);
      if (strlen(s)>0)
      {
        machine[nbOfMachines] = new POPString(s);
	      nbOfMachines++;
      }
    }
    fclose(f);
  } else
  {
    nbOfMachines=1;
    machine[0] = new POPString("localhost"); 
  }
  return nbOfMachines;
}

//--------------------------
int main(int argc, char **argv)
{ 
  // For statistical purpose
  int totA=0, totD=0, totS=0;
  double cTime, tTime;
  Timer timer;
  int D, A, S;
 
  // Check and get parameters	
  if (argc!=4)
  {
    printf("\nUsage: parocrun objmap %s NbWorkers NbNoeuds nbMaxVoisins\n\n",
            argv[0]);
    return 0;
  }

  int nbWorkers = atoi(argv[1]);  // Number of machines effectively used
  if (nbWorkers<1) nbWorkers = 1;

  // Variable to define the random graph
  int netSize = atoi(argv[2]);       // Size of the Graph
  int maxNeighbours = atoi(argv[3]); // Max nb. of neighbours
 
  // Check parameters consistency
  if ((netSize < 7) || (maxNeighbours > netSize))
  {
    printf("\nIncorrect parameters, exiting\n\n");
    exit(EXIT_FAILURE);
  }
  // End of paramters check

 // Get the available machines
  POPString* machine[nbMaxMachines];  
  int nbOfMachines = getAvailableMachines(MachinesList, machine);

  if (nbWorkers>nbOfMachines) nbWorkers = nbOfMachines;

  printf(" Run on %d machines\n", nbWorkers); 
  for (int i=0; i<nbWorkers; i++)
    printf("  %d : %s \n", i, (const char*)*(machine[i])); 

  ShortPathObject* theObjects[netSize];

  try
  {
    // Construct a random connected graph of netSize shortPathObjects
    // Each object having between 0 and maxNeighbours neighbours
    for (int i=0; i<netSize; i++)
    {
      theObjects[i] = new ShortPathObject(*(machine[i%nbWorkers]));
      theObjects[i]->setMyId(i);
    }

    for (int i=0; i<netSize; i++)
    {
      int nbNeighbours = (rand() % maxNeighbours) + 1;
      theObjects[i]->initComputation(netSize+1, nbNeighbours); 
      theObjects[i]->setNeighbours(0,*(theObjects[(i+1) % netSize]),1);
      int r;
      for (int j=1; j<nbNeighbours; j++)
      {
        do { r = (rand() % netSize); } while (r==i);
        theObjects[i]->setNeighbours(j, *(theObjects[r]), 1);
      }
    }

    printf("Starting computations for random graph (%d, %d)\n",
          netSize, maxNeighbours);

    timer.Start();
    theObjects[0]->startComputation();

    // Get statistical results
    cTime = theObjects[0]->getTime();
    tTime = timer.Elapsed();
    for (int i=0; i<netSize; i++)
    {
     theObjects[i]->getStatistic(D, A, S);
      totD = totD + D;
      totA = totA + A;
      totS = totS + S;
    }


	} //end try
																	   
  catch (POPException *e)
	{
		printf("\n !!! Exception occurs in application ");
		//POPSystem::perror(e);
		delete e;
		return 0;
	} // catch

 printf("MESSAGES: dist=%d ack=%d stop=%d, compute Time =%g, total Time=%g\n",
          totD, totA, totS, cTime, tTime);

  printf("The shortest paths are:\n");
  for (int i=0; i<netSize; i++)
    printf("%4d ",theObjects[i]->getPath());
  printf("\n");

  // Write results in Excel file (tab separated values)
  FILE *excelFile;

  excelFile = fopen(ExcelFileName,"r");
  if (excelFile==NULL)
  {
    excelFile = fopen(ExcelFileName,"w");
    if (excelFile!=NULL)
    {
      fprintf(excelFile,
      "Size\tMaxN\tW\tDistM\tAckM\tStopM\tComputeT\tTotalT\n");
      fclose(excelFile);
    }
    else printf("ERROR: unable to open result file:%s\n", ExcelFileName);
  } 
  excelFile = fopen(ExcelFileName,"a");
  if (excelFile!=NULL)
  {
    fprintf(excelFile,"%d\t%d\t%d\t%d\t%d\t%d\t%11.8g\t%11.8g\n",
    netSize, maxNeighbours, nbWorkers, totD, totA, totS, cTime, tTime);
    fclose(excelFile);
  }
  else printf("ERROR: unable to open result file:%s\n", ExcelFileName);

  // Delete the Graph
  for (int i=0; i<netSize; i++) delete theObjects[i];
  return 0;
}
