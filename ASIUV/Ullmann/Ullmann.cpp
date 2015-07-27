#include"Ullmann.h"
#include"TimeUtility.h"
#include"AdjacenceListsGRAPH_IO.h"
#include<iostream>
#include<vector>
#include<map>
#include<set>

using namespace std;


Ullmann::Ullmann(string dataGraphFileName, string queryGraphFileName) {

	Ullmann::dataGraphFileName = dataGraphFileName;
	Ullmann::queryGraphFileName = queryGraphFileName;

	std::ifstream dataGraphFile = std::ifstream(dataGraphFileName);
	std::ifstream queryGraphFile = std::ifstream(queryGraphFileName);

	if(!dataGraphFile.is_open()) {
		cout<<"data graph file doesn't exist"<<endl;
		exit(1);
	}
	if(!queryGraphFile.is_open()) {
		cout<<"query graph file doesn't exist"<<endl;
		exit(1);
	}
	AdjacenceListsGRAPH_IO::loadGraphFromFile(dataGraphFile, dataGraphVector);

	for(int dataGraphIndex = 0; dataGraphIndex < dataGraphVector.size(); dataGraphIndex ++) {
		dataGraphVector[dataGraphIndex].buildLabelVertexList();
	}

	AdjacenceListsGRAPH_IO::loadGraphFromFile(queryGraphFile, queryGraphVector);

	numberOfRecursiveCalls = 0;
	sufficientNumberOfEmbeddings = 1000; // only calculate 1000 embeddings for each query graph
	totalNumberOfEmbeddings = 0;
}

void Ullmann::execute() {

	for(int dataGraphIndex = 0; dataGraphIndex < dataGraphVector.size(); dataGraphIndex ++) {
		
		dataGraph = &dataGraphVector[dataGraphIndex];

		for(int queryGraphIndex = 0; queryGraphIndex < queryGraphVector.size(); queryGraphIndex ++){
			queryGraph = &queryGraphVector[queryGraphIndex];
			genericQueryProc();
			cout<<queryGraphIndex<<" : finish one query with embeddings :"<<newEmbeddingsForEachQuery<<endl;
		}
		cout<<dataGraphIndex<<" finish one data graph ******* "<<endl;
	}
}

void Ullmann::clean() {
	candidates.clear(); // clear the candidates
	newEmbeddingsForEachQuery = 0;
}

void Ullmann::genericQueryProc(){

	clean();

	/* filter candidates with a different label */
	filterCandidates();

	if(candidates.size() != queryGraph -> getNumberOfVertexes()) {
		return;
	}

	subgraphSearch();
}

//@Common
void Ullmann::filterCandidates() {

	map<int,vector<int>>::iterator candidateSetsIterator;

	vector<AdjacenceListsGRAPH::Vertex> * queryVertexLists = queryGraph -> getVertexList();
	map<int,vector<int>> * labelDataVertexList = dataGraph -> getLabelVertexList();


	for(vector<AdjacenceListsGRAPH::Vertex>::iterator queryVertexIterator = queryVertexLists->begin(); queryVertexIterator != queryVertexLists->end(); queryVertexIterator++){

		candidateSetsIterator = labelDataVertexList -> find(queryVertexIterator->label);
		
		if(candidateSetsIterator != labelDataVertexList -> end()) {
			candidates.insert(std::pair<int,std::vector<int> *>(queryVertexIterator->id, & candidateSetsIterator->second));
		}
		else{
			// to do ? the query has no embeddings
			return;
		}
	}
}

void Ullmann::subgraphSearch() {

	numberOfRecursiveCalls ++;

	if( embedding.size() == queryGraph -> getNumberOfVertexes()){
		// FIND AN EMBDDING // // To Do ?
		//showEmbedding();
		totalNumberOfEmbeddings ++;
		newEmbeddingsForEachQuery ++;
		return ;
	}

	AdjacenceListsGRAPH::Vertex u = nextQueryVertex();
	vector<int> * candidates_u = candidates.find(u.id) -> second;

	// For each v in C(u)
	for(vector<int>::iterator v = candidates_u -> begin(); v != candidates_u -> end(); v++) {

		if(newEmbeddingsForEachQuery >= sufficientNumberOfEmbeddings) {
			break; // only calculate 1000 embeddings for each query graph
		}

		//refine candidate
		if(!refineCandidates(u,*v)) {
			continue;
		}
		if(isJoinable(u.id, *v)) {
			updateState(u.id, *v);
			subgraphSearch();
			restoreState(u.id, *v);
		}
	}

}

AdjacenceListsGRAPH::Vertex Ullmann::nextQueryVertex() {
	return (queryGraph -> getVertexList())->at(embedding.size());
}

bool Ullmann::refineCandidates(AdjacenceListsGRAPH::Vertex & u, const int & v) {

	// Cannot map a node twice, TODO ? For hyber-node
	if(inverseEmbedding.find(v) != inverseEmbedding.end()) {
		return false;
	}
	// The candidates whose degree is smaller than the query vertex's will be filtered
	if(u.inDegree > dataGraph -> degree(v)) {
		return false;
	}
	return true;
}

bool Ullmann::isJoinable(int u, int v) {

	AdjacenceListsGRAPH::adjIterator adjIterator(queryGraph, u);

	for(AdjacenceListsGRAPH::link t = adjIterator.begin();  !adjIterator.end(); t=adjIterator.next()) {
		if(embedding.find(t->v) != embedding.end()){
			// u has an edge with query vertex t->v which has already been matched
			if( dataGraph -> edge( v, embedding.find(t->v)->second)){
				continue;
			}else{
				return false;
			}
		}
	}
	return true;
}

void Ullmann::updateState(int u, int v) {
	embedding.insert(std::pair<int,int>(u, v));
	inverseEmbedding.insert(std::pair<int,int>(v, u));
}

void Ullmann::restoreState(int u, int v){
	embedding.erase(u);
	inverseEmbedding.erase(v);
}



void Ullmann::showEmbedding() {
	std::cout<<"{";
	for(int i = 0; i < queryGraph -> getNumberOfVertexes(); i++){
		cout<<"Embedding: "<<i<<"->"<<embedding[i]<<" , ";
	}
	cout<<"}"<<endl;
}

// Getter and setter
long Ullmann::totalComputation(){
	return dataGraphVector.size() * queryGraphVector.size();
}

long Ullmann::totalEmbeddings(){
	return totalNumberOfEmbeddings;
}


long Ullmann::totalRecursiveCalls(){
	return numberOfRecursiveCalls;
}

int  Ullmann::totalNumberOfDataGraphs(){
	return dataGraphVector.size();
}
int  Ullmann::totalNumberOfQueryGraphs(){
	return queryGraphVector.size();
}