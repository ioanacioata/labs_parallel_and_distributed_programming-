// KColoring.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "KColoring.h"
#include <future>
#include<iostream>


using namespace std;

const int nrOfObjects = 5;

/*
1. (3.5p) Write a parallel (distributed or local, at your choice) program for solving the k-colouring problem. That is,
you are given a number k, and n objects and some pairs among them that have distinct colours. Find a solution
to colour them with at most c colours, if one exits.

Assume that you have a function that gets a vector with k
integers representing the assignment of colours to objects and check if the constraints are obeyed or not.
*/

bool isSafe(int currentVertex, bool graph[][10], int colours[], int currentColour) {
	cout << "current vertex: " << currentVertex << "  current colour: " << currentColour << endl;;
	for (int i = 0; i < nrOfObjects; i++) {
		if (graph[currentVertex][i] == true) {
			//if a neighbour is found 
			if (colours[i] == currentColour) {
				//and it has the same colour then is not safe
				return false;
			}
		}
	}
	return true;
}

bool graphColoringUtil(bool graph[][10], int nrOfColors, int colours[], int currentVertex) {

	if (currentVertex == nrOfObjects)
	{
		//then it's done 
		return true;
	}
	for (int c = 1; c <= nrOfColors; c++) {
		if (isSafe(currentVertex, graph, colours, c))
		{
			colours[currentVertex] = c;

			future<bool> future = async(
				[&graph, nrOfColors, &colours, currentVertex]() 
					{return graphColoringUtil(graph, nrOfColors, colours, currentVertex + 1); }
			);

			if (future.get()) {
				return true;
			}
			colours[currentVertex] = 0;
		}
	}
	return false;
}

//V=10
bool graphColoring(bool graph[][10], int nrOfColors) {
	int *colours = new int[nrOfObjects];
	for (int i = 0; i < nrOfObjects; i++) {
		colours[i] = 0;
	}
	if (graphColoringUtil(graph, nrOfColors, colours, 0) == false) {
		cout<<"Solution does not exist"<<endl;
		return false;
	}
	for (int c = 0; c < nrOfObjects; c++) {
		cout <<"v"<< c <<" -> "<< colours[c] << ", ";
	}
	cout<<"OK "<<endl;
	return true;
}


int main()
{
	bool graph[][10] = {
		{ false, true, true, true, false },
		{ true, false, false, true, true },
		{ true, false, false, false , true},
		{ true, true, false, false, true },
		{ false, true, true, true, false },
	};

	graphColoring(graph, 3);
	return 0;
}

