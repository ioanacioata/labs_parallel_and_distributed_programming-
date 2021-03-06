// MergeSort.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdafx.h"
#include "mpi.h"
#include <iostream>
#include <vector>
#include <time.h>
#include <stdint.h>

using namespace std;

const int tagSize = 1;
const int tagData = 2;
const int tagResult = 3;



void generate(vector<int>& v, size_t n) {
	v.reserve(n);
	for (size_t i = 0; i < n; ++i) {
		v.push_back((i * 101011) % 123456);
	}
}




bool isSorted(vector<int> const & v) {
	size_t const n = v.size();
	for (size_t i = 1; i < n; ++i) {
		if (v[i - 1] > v[i]) {
			return false;
		}
	}

	return true;
}

//Local merging of sorted sequences.
//Merges the sequences from begin1 to end1 with that from begin2 to end2.
//The result is stored starting at merged(which must have enough space)

void merge(int* begin1, int* end1, int* begin2, int* end2, int*merged) {
	int* curr1 = begin1;
	int* curr2 = begin2;

	while (curr1 < end1 || curr2 < end2) {
		if (curr2 >= end2 || (curr1 < end1 && *curr1 < *curr2)) {
			*merged = *curr1;
			++curr1;
		}
		else {
			*merged = *curr2;
			++curr2;
		}

		++merged;
	}
}


//Recursively executes merge-sort for the sequence of sequence from in and of size n.
//If nrProcs=1, all happens locally.
//Otherwise, half of the input sequence is sent to a child process for sorting and the other half is sorted recursively calling mergeSortRec()

void mergeSortRec(size_t n, int* in, size_t me, size_t nrProcs) {
	//in -  address of first element in vector of elements

	//if size of the array is <1 , exit
	if (n <= 1) {
		return;
	}

	size_t k = n / 2;
	if (nrProcs >= 2) {
		size_t child = me + nrProcs / 2;
		int sizes[2];
		sizes[0] = n - k;
		sizes[1] = nrProcs - nrProcs / 2;

		cout << "Worker " << me << ", sending to child " << child << " , part size = " << n - k << ", nrProcs = " << sizes[1] << "\n";

		//First send number of numbers to send and the number of processes that can be used by the child
		MPI_Send(sizes, 2, MPI_INT, child, tagSize, MPI_COMM_WORLD);
		MPI_Send(in + k, n - k, MPI_INT, child, tagData, MPI_COMM_WORLD);
		mergeSortRec(k, in, me, nrProcs / 2);
		MPI_Status status;
		MPI_Recv(in + k, n - k, MPI_INT, child, tagResult, MPI_COMM_WORLD, &status);
		cout << "Worker " << me << ", received from child " << child << ", part size = " << n - k << "\n";


	}
	else {
		//if one process, perform local mergeSort(normal merge sort)
		mergeSortRec(k, in, me, 1);
		mergeSortRec(n - k, in + k, me, 1);
	}

	int* buf = new int[n];
	//in - start index of list1
	//in + k - start index of list2
	merge(in, in + k, in + k, in + n, buf);
	for (size_t i = 0; i < n; ++i) {
		in[i] = buf[i];
	}
	delete[] buf;
}


//Called only by process 0.
void mergeSort(vector<int> &v, size_t nrProcs) {
	mergeSortRec(v.size(), v.data(), 0, nrProcs);
}



//Main function to be executed on the worker.
//It receives a vector from the parent, sorts it,
//possibly by using subordinate processes, and sends the result back to the parent.
void mergeWorker(size_t me) {
	//First, receive the number of numbers to sort and the numbers of sub-processes to use;
	//The sender is considered the parent.
	int sizes[2];
	MPI_Status status;
	MPI_Recv(sizes, 2, MPI_INT, MPI_ANY_SOURCE, tagSize, MPI_COMM_WORLD, &status);

	int parent = status.MPI_SOURCE;
	size_t n = sizes[0];
	size_t nrProcs = sizes[1];
	cout << "Worker " << me << ", child of " << parent << ", part size = " << n << " , nrProcs = " << nrProcs << "\n";

	//Receive data from the parent
	vector<int> v;
	v.resize(n);
	MPI_Recv(v.data(), n, MPI_INT, parent, tagData, MPI_COMM_WORLD, &status);
	cout << "Worker " << me << ", received from parent " << parent << ", part size = " << n << "\n";

	//Do the local sorting
	mergeSortRec(v.size(), v.data(), me, nrProcs);

	//Send back result to the parent
	cout << "Worker " << me << ", sending to parent " << parent << ", part size = " << n << "\n";
	MPI_Ssend(v.data(), n, MPI_INT, parent, tagResult, MPI_COMM_WORLD);

}






int main(int argc, char *argv[])
{

	//MPI_Init(&argc, &argv);
	MPI_Init(0, 0);
	int me;
	int nrProcs;
	MPI_Comm_size(MPI_COMM_WORLD, &nrProcs);
	MPI_Comm_rank(MPI_COMM_WORLD, &me);

	unsigned n;
	vector<int> v;

	if (argc != 2 || 1 != sscanf_s(argv[1], "%u", &n)) {
		fprintf(stderr, "usage: mergesort <n>\n");
		return 1;
	}

	if (me == 0) {
		generate(v, n);
		cout << "Array Generated.\n";
		mergeSort(v, nrProcs);
	}
	else {
		mergeWorker(me);
	}

	if (me == 0) {
		cout << ((n == v.size() && isSorted(v)) ? "OK!" : "Wrong!") << "\n";
	}


	MPI_Finalize();
	return 0;
}
