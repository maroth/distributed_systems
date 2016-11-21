#include <stdio.h>
#include <stdlib.h>
#include "shortpathobject.ph"

#define ExcelFileName  "Excel.txt"
#define nbMaxMachines 200
#define MachinesList "machines.ip"

//-------------------------------------------------------------
// Read  the file 'fileName' containing the list of machines,
// fill the array 'machines' and returns the number of machines
int getAvailableMachines(char* fileName, POPString* machine[])
{
  int nbOfMachines = 0;
  FILE* f;
  if ((f = fopen(fileName, "r"))!=NULL)
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
  }
  else
  {
    nbOfMachines=1;
    machine[0] = new POPString("localhost"); 
  }
  return nbOfMachines;
}


//------------------------------
int main(int argc, char **argv)
{
  // For statistical purpose
  int totA=0, totD=0, totS=0;
  double cTime, tTime;
  Timer timer;
  int D, A, S;  
 
  // Check and get parameters	 
  if (argc!=3)
  {
    printf("Usage: parocrun objmap %s nbWorkers size2Dtorus\n", argv[0]);
    return 0;
  }

  int nbWorkers = atoi(argv[1]);  // Number of machines effectively used
  if (nbWorkers<1) nbWorkers = 1;

  int torusSize = atoi(argv[2]);

  // Check parameters consistency
  if (torusSize<2)
  {
    printf("Size of the torus must be >1, exiting\n");
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

  ShortPathObject* theObjects[torusSize][torusSize];

  try
  {
    //------- Construct a square 2D torus of size torusSize -------
    // Create the nodes
    int nbNeighbours = 4;

    for (int i=0; i<torusSize; i++)
      for (int j=0; j<torusSize; j++)
      {
        theObjects[i][j] = new
           ShortPathObject(*(machine[(i+j*torusSize)%nbWorkers]));
        theObjects[i][j]->setMyId(i+j*torusSize);
        theObjects[i][j]->initComputation(torusSize*torusSize+1, nbNeighbours);
      }

    for (int i=0; i<torusSize; i++)
      for (int j=0; j<torusSize; j++)
      {
        theObjects[i][j]->
          setNeighbours(0,*(theObjects[i][(j-1+torusSize)%torusSize]),1);
        theObjects[i][j]->        
          setNeighbours(1,*(theObjects[i][(j+1)%torusSize]),1);
        theObjects[i][j]->
          setNeighbours(2,*(theObjects[(i+1)%torusSize][j]),1);
        theObjects[i][j]->
          setNeighbours(3,*(theObjects[(i-1+torusSize)%torusSize][j]),1);
      }

    printf("Computations for torus 2D of size %d\n", torusSize);
    timer.Start();
    theObjects[0][0]->startComputation();
    //theObjects[torusSize/2][torusSize/2]->startComputation();

    // Get statistical results
    //cTime = theObjects[0][0]->getTime();
    cTime = theObjects[torusSize/2][torusSize/2]->getTime();
    tTime = timer.Elapsed();
    timer.Stop();

    for (int i=0; i<torusSize; i++)
      for (int j=0; j<torusSize; j++) 
      {
        theObjects[i][j]->getStatistic(D, A, S);
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
  for (int i=0; i<torusSize; i++)
  {
    for (int j=0; j<torusSize; j++)
    { printf("%4d ",theObjects[i][j]->getPath());}
    printf("\n");
  }

  // Write results in Excel file (tab separated values)
  FILE *excelFile;

  excelFile = fopen(ExcelFileName,"r");
  if (excelFile==NULL)
  {
    excelFile = fopen(ExcelFileName,"w");
    if (excelFile!=NULL)
    {
      fprintf(excelFile, "Size\tW\tDistM\tAckM\tstopM\tComputeT\tTotalT\n");
      fclose(excelFile);
    }
    else printf("ERROR: unable to open result file:%s\n", ExcelFileName);
  } 
  excelFile = fopen(ExcelFileName,"a");
  if (excelFile!=NULL)
  {
    fprintf(excelFile,"%d\t%d\t%d\t%d\t%d\t%g\t%g\n",
              torusSize, nbWorkers, totD, totA, totS, cTime, tTime);
    fclose(excelFile);
  }
  else printf("ERROR: unable to open result file:%s\n", ExcelFileName);

  // Delete the Torus
  for (int i=0; i<torusSize; i++)
    for (int j=0; j<torusSize; j++)
      delete theObjects[i][j];
 
    return 0;
}
