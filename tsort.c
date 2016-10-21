#include <sys/timeb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define INSERTION_SORT 'I'
#define QUICK_SORT 'Q'
#define TRUE 1
#define FALSE 0
//Loop through for each thread
#define FOR_EACH_THREAD for(i = 0; i < noOfThreads ; i++ )

//Loop through for each element
#define FOR_EACH_ELEMENT for(i = 0; i < noOfElements ; i++ )

//Print each element present in the array for debugging
#define PRINT_EACH_ELEMENT printf("\nThe Elements Are:\n "); \
		FOR_EACH_ELEMENT \
		printf(" %d ",elements[i]); \
		printf("\n");

//Structure which contains information regarding the length of the section of the elements to be sorted and the elements
struct sort_data {
	//Completed index
	int completion;
	//Number of elements from completion
	int range;
	//Sorted data
	int *sorteddata;
};

typedef struct sort_data SORT_DATA;

long gRefTime;
//No of threads
int noOfThreads;
//No of Elements
int noOfElements;
//Sorting Algorithm
char sortAlgo;
//Size of each section to sort
int range;
//Size of the last section to sort
int lastRange;
//The array of elements
int *elements;

int Rand(int, int);
void Swap(int *, int *);
int Partition(int, int);
void *RunInsertionSort(void *);
void InsertionSort(SORT_DATA *);
void *RunQuickSort(void *);
void QuickSort(int, int);
void FillArray();
int AssertTest();

long GetMilliSecondTime(struct timeb timeBuf) {
	long mScdTime;

	mScdTime = timeBuf.time;
	mScdTime *= 1000;
	mScdTime += timeBuf.millitm;
	return mScdTime;
}

long GetCurrentTime(void) {
	long crntTime = 0;
	struct timeb timeBuf;
	ftime(&timeBuf);
	crntTime = GetMilliSecondTime(timeBuf);

	return crntTime;
}

void SetTime(void) {
	gRefTime = GetCurrentTime();
}

long GetTime(void) {
	long crntTime = GetCurrentTime();
	return (crntTime - gRefTime);
}

//Function to call Insertion Sort
void *RunInsertionSort(void *sample) {
	if (sample != NULL) {
		SORT_DATA *data;
		data = (SORT_DATA *) sample;
		//Call InsertionSort
		InsertionSort(data);
		//Copy sorted data from the element array to dynamic array in data
		memcpy(data->sorteddata, elements + (data->completion),
				sizeof(int) * (data->range));
	} else
		exit(EXIT_FAILURE);
	//Exit thread
	pthread_exit(0);
}

//Insertion Sort Logic
void InsertionSort(SORT_DATA *data) {
	if (data != NULL) {
		int i, j, temp;
		for (i = data->completion + 1; i < (data->completion + data->range);
				i++) {
			temp = elements[i];
			for (j = i - 1; j >= data->completion && elements[j] > temp; j--)
				elements[j + 1] = elements[j];
			elements[j + 1] = temp;
		}
	} else
		exit(EXIT_FAILURE);
}

//Function to call Quick Sort
void *RunQuickSort(void *sample) {
	if (sample != NULL) {
		SORT_DATA *data;
		data = (SORT_DATA *) sample;
		//Call QuickSort
		QuickSort(data->completion, (data->completion + data->range) - 1);
		//Copy sorted data from the element array to dynamic array in data
		memcpy(data->sorteddata, elements + (data->completion),
				sizeof(int) * (data->range));
	} else
		exit(EXIT_FAILURE);
	//Exit Thread
	pthread_exit(0);
}

//Quick Sort Logic
void QuickSort(int p, int r) {
	if (p >= r)
		return;
	if (p > noOfElements || p < 0 || r < 0 || r > noOfElements)
			exit(EXIT_FAILURE);

	int q;
	q = Partition(p, r);
	QuickSort(p, q - 1);
	QuickSort(q + 1, r);
}

//Partition Logic
int Partition(int p, int r) {
	if (p > noOfElements || p < 0 || r < 0 || r > noOfElements)
		exit(EXIT_FAILURE);
	int i, j, x, pi;
	pi = Rand(p, r);
	Swap(&elements[r], &elements[pi]);

	x = elements[r];
	i = p - 1;
	for (j = p; j < r; j++) {
		if (elements[j] < x) {
			i++;
			Swap(&elements[i], &elements[j]);
		}
	}
	Swap(&elements[i + 1], &elements[r]);
	return i + 1;
}

//Swap Logic
void Swap(int *x, int *y) {
	if (x != NULL && y != NULL) {
		int temp = *x;
		*x = *y;
		*y = temp;
	} else
		exit(EXIT_FAILURE);
}

int Rand(int x, int y) {
	int range = y - x + 1;
	int r = rand() % range;
	r += x;
	return r;
}

// Merging the sorted arrays back together in O(N) complexity
void Merge(int *sections[], int *originalArray) {
	if (sections != NULL && originalArray != NULL) {
		int i, j, completed = 0, max = lastRange, temp, index;
		//Set of indices of the each of the sections
		int sectionPointers[noOfThreads];
		//Set the indices to 0
		for (i = 0; i < noOfThreads; i++)
			sectionPointers[i] = 0;
		j = 0;

		//fill the original array with all the elements
		while (j < noOfElements) {
			//initialise the temp to the first available element of the remaining sections
			temp = sections[completed][sectionPointers[completed]];
			index = completed;
			//Iterate through all the sections and pull out the smallest number from the remaining sections
			for (i = 0; i < noOfThreads; i++) {
				int size = (i == noOfThreads - 1) ? lastRange : range;
				//Check the section only if it is not exhausted.
				if (sectionPointers[i] < size) {
					temp = (i == 0) ? sections[i][sectionPointers[i]] : temp;
					//Compare for the smaller elements
					if (temp > sections[i][sectionPointers[i]]) {
						//Save the smaller element
						temp = sections[i][sectionPointers[i]];
						//Save the section number
						index = i;
					}
				}
			}
			//Write the smallest element in the original element
			originalArray[j++] = temp;
			//Update the section index
			sectionPointers[index]++;
			//Check if the first of previously available sections is exhausted
			max = (index == noOfThreads - 1) ? lastRange : range;
			if (completed == index) {
				//update to the first available section
				while (sectionPointers[completed] >= max) {
					completed++;
					max = (completed == noOfThreads - 1) ? lastRange : range;
				}
			}
		}
	} else
		exit(EXIT_FAILURE);
}

//Fill the array with <noOfElements> random elements
void FillArray(int *dataArray) {
	if (dataArray != NULL) {
		int i;
		FOR_EACH_ELEMENT
			dataArray[i] = rand() % 100;
	} else
		exit(EXIT_FAILURE);
}

//Logic to check if the array has been sorted with O(N) complexity
int AssertTest() {
	int i;
	FOR_EACH_ELEMENT
	{
		if (i == noOfElements - 1)
			break;
		if (elements[i] > elements[i + 1])
			return FALSE;
	}
	return TRUE;
}

int main(int argc, char *argv[]) {
	int **sections;
	int completion;
	int i;
	SORT_DATA data;

	SetTime();
	if (argc != 4) {
		fprintf(stderr,
				"usage: tsort <number_of_elements> <number_of_thread> <sort_algorithm: I|Q>\n");
		exit(EXIT_FAILURE);
	}
	if (argv[3][0] != 'I' && argv[3][0] != 'Q') {
		fprintf(stderr, "%c must be either I or Q\n", argv[3][0]);
		exit(EXIT_FAILURE);
	}

	noOfElements = atoi(argv[1]);
	noOfThreads = atoi(argv[2]);

	if (noOfElements <= 0) {
		fprintf(stderr, "Minimum <number_of_elements> allowed is 1\n");
		exit(EXIT_FAILURE);
	}

	if (noOfThreads > noOfElements) {
		fprintf(stderr,
				"<number_of_elements> must be more or equal than the <number_of_thread>\n");
		exit(EXIT_FAILURE);
	}
	if (noOfThreads > 16) {
		fprintf(stderr, "Maximum <number_of_thread> allowed is 16\n");
		exit(EXIT_FAILURE);
	}
	if (noOfThreads <= 0) {
		fprintf(stderr, "Minimum <number_of_thread> allowed is 1\n");
		exit(EXIT_FAILURE);
	}

	SORT_DATA dataSet[noOfThreads];
	sortAlgo = argv[3][0];
	range = noOfElements / noOfThreads;
	lastRange = range + (noOfElements % noOfThreads);
	sections = (int **) malloc(noOfThreads * sizeof(int *));
	for (i = 0; i < noOfThreads - 1; i++)
		sections[i] = (int *) calloc(range, sizeof(int));
	sections[noOfThreads - 1] = (int *) calloc(lastRange, sizeof(int));
	elements = (int *) malloc(sizeof(int) * noOfElements);
	pthread_t tid[noOfThreads];
	pthread_attr_t attr;
	data.sorteddata = NULL;

	switch (sortAlgo) {
	case INSERTION_SORT:
		FillArray(elements);
		SetTime();
		completion = 0;
		FOR_EACH_THREAD
		{
			data.completion = completion;
			if (i < noOfThreads - 1)
				data.range = range;
			else
				data.range = lastRange;
			//Run InsertionSort Sequentially
			InsertionSort(&data);
			//Copy the sorted data from elements to a section
			memcpy(sections[i], elements + (data.completion),
					sizeof(int) * (data.range));
			completion = completion + range;
		}
		//Merge the sections to the original array
		Merge(sections, elements);
		printf("The time for sequential Insertion Sort: %ld\n", GetTime());
		//Check if the elements are sorted properly
		printf("%s\n", AssertTest() ? "Correct" : "Incorrect");
		//Refill the array
		FillArray(elements);
		SetTime();
		completion = 0;
		//Initialize the thread attribute
		pthread_attr_init(&attr);

		FOR_EACH_THREAD
		{
			//set limits and memory references for each of the sections to be passed to the threads
			dataSet[i].completion = completion;
			if (i < noOfThreads - 1)
				dataSet[i].range = range;
			else
				dataSet[i].range = lastRange;
			//Refer to the same address as sections
			dataSet[i].sorteddata = sections[i];
			completion = completion + range;
		}
		//Create the threads and run Insertion Sort by each one of them
		FOR_EACH_THREAD
		{
			if (pthread_create(&tid[i], &attr, RunInsertionSort, &dataSet[i])) {
				fprintf(stderr, "Error creating thread\n");
				free(sections);
				free(elements);
				sections = NULL;
				elements = NULL;
				exit(EXIT_FAILURE);
			}

		}
		//Make the parent thread wait for all the child threads
		FOR_EACH_THREAD
		{
			if (pthread_join(tid[i], NULL)) {
				fprintf(stderr, "Error joining thread\n");
				free(sections);
				free(elements);
				sections = NULL;
				elements = NULL;
				exit(EXIT_FAILURE);
			}
		}
		//Merge the sections to the original array
		Merge(sections, elements);
		printf("The time for Multi-Threaded Insertion Sort: %ld\n", GetTime());
		//PRINT_EACH_ELEMENT
		printf("%s\n", AssertTest() ? "Correct" : "Incorrect");
		break;

	case QUICK_SORT:
		FillArray(elements);
		//PRINT_EACH_ELEMENT
		SetTime();
		completion = 0;
		FOR_EACH_THREAD
		{
			data.completion = completion;
			if (i < noOfThreads - 1)
				data.range = range;
			else
				data.range = lastRange;
			//Run QuickSort Sequentially
			QuickSort(data.completion, (data.completion + data.range) - 1);
			//Copy the sorted data from elements to a section
			memcpy(sections[i], elements + (data.completion),
					sizeof(int) * (data.range));
			completion = completion + range;
		}
		//Merge the sections to the original array
		Merge(sections, elements);
		printf("The time for sequential Quick Sort: %ld\n", GetTime());
		printf("%s\n", AssertTest() ? "Correct" : "Incorrect");
		//PRINT_EACH_ELEMENT
		FillArray(elements);
		//PRINT_EACH_ELEMENT
		SetTime();
		completion = 0;
		//Initialize the thread attribute
		pthread_attr_init(&attr);

		FOR_EACH_THREAD
		{
			dataSet[i].completion = completion;
			if (i < noOfThreads - 1)
				dataSet[i].range = range;
			else
				dataSet[i].range = lastRange;
			//Copy the memory location of section to sorteddata
			dataSet[i].sorteddata = sections[i];
			//Copy the sorted data from elements to a section
			if (pthread_create(&tid[i], &attr, RunQuickSort, &dataSet[i])) {
				fprintf(stderr, "Error creating thread\n");
				free(sections);
				free(elements);
				sections = NULL;
				elements = NULL;
				exit(EXIT_FAILURE);
			}
			completion = completion + range;
		}
		//Make the parent thread wait for all the child threads
		FOR_EACH_THREAD
			if (pthread_join(tid[i], NULL)) {
				fprintf(stderr, "Error joining thread\n");
				free(sections);
				free(elements);
				sections = NULL;
				elements = NULL;
				exit(EXIT_FAILURE);
			}
		//Merge the sections to the original array
		Merge(sections, elements);
		printf("The time for Multi-Threaded Quick sort: %ld\n", GetTime());
		printf("%s\n", AssertTest() ? "Correct" : "Incorrect");
		//PRINT_EACH_ELEMENT
		break;

	default:
		fprintf(stderr,
				"<sort_algorithm> parameter can only be either I (for Insertion Sort) or Q (for Quick Sort)\n");
		break;
	}

	if (data.sorteddata != NULL) {
		free(data.sorteddata);
		data.sorteddata = NULL;
	}
	free(sections);
	sections = NULL;
	free(elements);
	elements = NULL;
	exit(EXIT_SUCCESS);
}

