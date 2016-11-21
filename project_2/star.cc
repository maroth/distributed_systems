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
  if (argc < 3)
  {
    printf("\nusage: parocrun objmap %s nbWorkers size\n\n",
           argv[0]);
    exit(EXIT_FAILURE);
  }

  int nbWorkers = atoi(argv[1]);  // Number of machines effectively used
  if (nbWorkers<1) nbWorkers = 1;

  // Variable to define the Star
  int n = atoi(argv[2]);    // size of the star
  
  // Check parameters consistency
  if ((n < 2))
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

  ShortPathObject* theStar[n];

  try
  {
    //------ Construct a Star of size n ---------
    // Create the nodes
    for (int i=0; i<n; i++)
    {
      theStar[i] = new ShortPathObject(*(machine[i%nbWorkers]));
      theStar[i]->setMyId(i);
    }

    // Connect the nodes
    theStar[0]->initComputation(n+1, n-1);
    for (int i=1; i<n; i++)
    {
      theStar[0]->setNeighbours(i-1,*(theStar[i]),1);
      theStar[i]->initComputation(n, 0);//1);
      //theStar[i]->setNeighbours(0,*(theStar[0]),1);
    }

    // Start the computation 
    printf("Computation for Star: N=%d,\n",n);
    timer.Start();
    theStar[0]->startComputation();

    // Get statistical results
    cTime = theStar[0]->getTime();
    tTime = timer.Elapsed();
    for (int i=0; i<n; i++)
    {
      theStar[i]->getStatistic(D, A, S);
      totD = totD + D;
      totA = totA + A;
      totS = totS + S;
    }

	} //end try
																	   
  catch (POPException *e)
	{
		printf("\n !!! Exception occurs in application \n");
		POPSystem::perror(e);
		delete e;
		return 0;
	} // catch

  printf("MESSAGES: dist=%d ack=%d stop=%d, compute Time =%g, total Time=%g\n",
          totD, totA, totS, cTime, tTime);

  printf("The shortest paths are:\n");
  for (int i=0; i<n; i++)
    printf("%4d ",theStar[i]->getPath());
  printf("\n");

  // Write results in Excel file (tab separated values)
  FILE *excelFile;

  excelFile = fopen(ExcelFileName,"r");
  if (excelFile==NULL)
  {
    excelFile = fopen(ExcelFileName,"w");
    if (excelFile!=NULL)
    {
      fprintf(excelFile,"Size\t");  
      fprintf(excelFile,"W\tDistM\tAckM \tStopM\tComputeT\tTotalT\n");
      fclose(excelFile);
    }
    else printf("ERROR: unable to open result file:%s\n", ExcelFileName);
  } 
  excelFile = fopen(ExcelFileName,"a");
  if (excelFile!=NULL)
  {
    fprintf(excelFile,"%d\t", n);  
    fprintf(excelFile,"%d\t%d\t%d\t%d\t%11.8g\t%11.8g\n",
                      nbWorkers, totD, totA, totS, cTime, tTime);
    fclose(excelFile);
  }
  else printf("ERROR: unable to open result file:%s\n", ExcelFileName);

  // Delete the Star
  for (int i=0; i<n; i++) delete theStar[i];
  return 0;
}
