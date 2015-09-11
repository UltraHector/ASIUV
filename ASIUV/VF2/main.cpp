#include"VF2.h"
#include"InputCommandLineParser.h"
#include<iostream>
#include<fstream>
#include"TimeUtility.h"

using namespace std;


/*  Global members */
string dataGraphFileName, queryGraphFileName, outputFileName;
double timeClapse;
long totalNumberOfComputations;
long totalRecursiveCalls;
long numberOfEmbeddings;
int numberOfDataGraphs;
int numberOfQueryGraphs;
/*  Function declarations */
void help();
void finalResult();

int main(int argc, char* argv[]) {


	/*dataGraphFileName = "C:/Users/s2813995/Desktop/QueryProcessingHouse/human/human.hgraph";
	queryGraphFileName = "C:/Users/s2813995/Desktop/QueryProcessingHouse/human/human_q8.graph";*/
	if(InputCommandLineParser::cmdOptionExists(argc, argv, "-d")) { 
		dataGraphFileName = InputCommandLineParser::getCmdOption(argc, argv, "-d");
	}
	if(InputCommandLineParser::cmdOptionExists(argc, argv, "-q")) {
		queryGraphFileName = InputCommandLineParser::getCmdOption(argc, argv, "-q");
	}
	
	if(InputCommandLineParser::cmdOptionExists(argc, argv, "-p")) {
		outputFileName = InputCommandLineParser::getCmdOption(argc, argv, "-p");
	}
	if(!InputCommandLineParser::cmdOptionExists(argc, argv, "-q") || !InputCommandLineParser::cmdOptionExists(argc, argv, "-d")){
		cout<<"Wrong Parameters"<<endl;
		help();
		exit(1);
	}
	
	

	VF2 vf2(dataGraphFileName, queryGraphFileName);

	// start to record the time
	TimeUtility::StartCounterMill();
	vf2.execute();
	// end record the time
	timeClapse = TimeUtility::GetCounterMill();

	


	// output final result
	totalRecursiveCalls = vf2.totalRecursiveCalls();
	totalNumberOfComputations = vf2.totalComputation();
	numberOfEmbeddings = vf2.totalEmbeddings();
	numberOfDataGraphs = vf2.totalNumberOfDataGraphs();
	numberOfQueryGraphs = vf2.totalNumberOfQueryGraphs();
	
	finalResult(); 
}

void help(){
	cout<<"Options: "<<endl;
	cout<<"-d  The datagraph file"<<endl;
	cout<<"-q  The querygraph file"<<endl;
}

void finalResult() {
	std::ofstream resultFile = std::ofstream(outputFileName, std::ios_base::app);
	resultFile << "***" <<dataGraphFileName << "***" <<queryGraphFileName<<endl;
	//std::ofstream resultFile = std::ofstream("C:/Users/s2813995/Desktop/QueryProcessingHouse/data_query_2.result");
	double avgTimeCost = timeClapse / totalNumberOfComputations;
	long avgRecursiveCall = totalRecursiveCalls / totalNumberOfComputations;
	resultFile << "Total Number of data graph is: " <<  numberOfDataGraphs <<endl;
	resultFile << "Total Number of query graph is: " <<  numberOfQueryGraphs <<endl;
	resultFile << "Total Number of computations is: " <<  totalNumberOfComputations <<endl;
	resultFile << "Avg Number of time cost is: " <<  avgTimeCost <<endl;
	resultFile << "Avg Number of recursive call is: " <<  avgRecursiveCall <<endl;
	resultFile << "Total Number of embeddings: " <<  numberOfEmbeddings << endl;
}