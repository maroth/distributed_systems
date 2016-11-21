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
    printf("\nusage: parocrun objmap %s nbWorkers size dimension k1 k2 ...\n\n",
           argv[0]);
    exit(EXIT_FAILURE);
  }

  int nbWorkers = atoi(argv[1]);  // Number of machines effectively used
  if (nbWorkers<1) nbWorkers = 1;

  // Variable to define the K-Ring
  int n, k;    // n= size, k=dimension
  int* ki;     // list og K values

  n = atoi(argv[2]);    // size of the Kring
  k = atoi(argv[3]);    // dimension of the Kring
  
  // Check parameters consistency
  if ((n < 5) || (k < 1))
  {
    printf("\nIncorrect parameters, exiting\n\n");
    exit(EXIT_FAILURE);
  }

  ki=(int*)malloc(k * sizeof(int)); // list of Ki
  ki[0] = 1;                        // the first K is always 1

  //get list of Ki from command line
  for (int i=1; i<k; i++)
    ki[i]=atoi(argv[3+i]);

  // End of paramters check

 // Get the available machines
  POPString* machine[nbMaxMachines];  
  int nbOfMachines = getAvailableMachines(MachinesList, machine);

  if (nbWorkers>nbOfMachines) nbWorkers = nbOfMachines;

  printf(" Run on %d machines\n", nbWorkers); 
  for (int i=0; i<nbWorkers; i++)
    printf("  %d : %s \n", i, (const char*)*(machine[i])); 

  ShortPathObject* theKring[n];

  try
  {
    //------ Construct a K-Ring of size n ---------
    // Create the nodes
    for (int i=0; i<n; i++)
    {
      theKring[i] = new ShortPathObject(*(machine[i%nbWorkers]));
      theKring[i]->setMyId(i);
    }

    // Connect the nodes
    for (int i=0; i<n; i++)
    {
      theKring[i]->initComputation(n+1, 2*k); 
      for (int j=0; j<(2*k); j=j+2)
      {
        theKring[i]->setNeighbours(j, *(theKring[(i+ki[j/2])%n]), 1 );
        theKring[i]->setNeighbours(j+1, *(theKring[(i+n-ki[j/2])%n]), 1 );
      }
    }

    // Start the computation 
    printf("Computation for K-Ring: N=%d, K=%d (1",n,k);
    for (int i=1; i<k; i++) printf(", %d", ki[i]); printf(") ");
    timer.Start();
    theKring[0]->startComputation();

    // Get statistical results
    cTime = theKring[0]->getTime();
    tTime = timer.Elapsed();
    for (int i=0; i<n; i++)
    {
      theKring[i]->getStatistic(D, A, S);
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
    printf("%4d ",theKring[i]->getPath());
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
	    for (int i = 0; i < k; i++) fprintf(excelFile,"K%d\t", i); 
      fprintf(excelFile,"W\tDistM\tAckM \tStopM\tComputeT\tTotalT\n");
      fclose(excelFile);
    }
    else printf("ERROR: unable to open result file:%s\n", ExcelFileName);
  } 
  excelFile = fopen(ExcelFileName,"a");
  if (excelFile!=NULL)
  {
    fprintf(excelFile,"%d\t", n); 
	  for (int i = 0; i < k; i++) fprintf(excelFile,"%d\t", ki[i]); 
    fprintf(excelFile,"%d\t%d\t%d\t%d\t%11.8g\t%11.8g\n",
                      nbWorkers, totD, totA, totS, cTime, tTime);
    fclose(excelFile);
  }
  else printf("ERROR: unable to open result file:%s\n", ExcelFileName);

  // Delete the K-Ring
  for (int i=0; i<n; i++) delete theKring[i];
  return 0;
}
