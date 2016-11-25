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

	ShortPathObject* bTree[n];
	int leafArr[n];
	memset(leafArr, 0, (n -1));
	int leafCnt = 0;

	try
	{
		//------ Construct a binary tree of size n ---------
		// Create the nodes
		for (int i=0; i<n; i++)
		{
			bTree[i] = new ShortPathObject(*(machine[i%nbWorkers]));
			bTree[i]->setMyId(i);
		}

		// Connect the nodes
		
		for (int i=0; i<n; i++)
		{
			if((n-i-1) >= 2) { // has two children
				bTree[i]->initComputation(n, 2);
				bTree[i]->setNeighbours(0, *(bTree[i+1]), 1);
				bTree[i]->setNeighbours(1, *(bTree[i+2]), 1);
			} else if ((n-i-1) == 1) { // has one child
				bTree[i]->initComputation(n, 1);
				bTree[i]->setNeighbours(0, *(bTree[i+1]), 1);
			} else { // isLeaf
				//bTree[i]->initComputation(n, 0);
				bTree[i]->initComputation(n, 1);
				leafArr[leafCnt++] = i;
			}
		}

		for(int i = 0; i<leafCnt - 1; i++) 
		{
			bTree[leafArr[i]]->setNeighbours(0, *(bTree[leafArr[i+1]]), 1);
		}
		bTree[leafArr[leafCnt - 1]]->setNeighbours(0, *(bTree[leafArr[0]]), 1);
  

		// Start the computation 
		printf("Computation for Binary Tree: N=%d,\n",n);
		timer.Start();
		bTree[0]->startComputation();

		// Get statistical results
		cTime = bTree[0]->getTime();
		tTime = timer.Elapsed();
		for (int i=0; i<n; i++)
		{
			bTree[i]->getStatistic(D, A, S);
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
		printf("%4d ",bTree[i]->getPath());
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
	for (int i=0; i<n; i++) delete bTree[i];
	return 0;
}
