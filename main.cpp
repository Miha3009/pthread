#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>

using namespace std;

int threads_count;
int task_size;

int* table;
pthread_mutex_t mutex;
string str;
char* res;
int symbols_in_process;

struct task
{
	int size;
	int start;
};

void gen_table(){
	table = new int[128];
	for(int i = 0; i < 128; i++) table[i] = 0;

	unordered_set<int> keys;
	keys.insert(0);
	int key = 0;

	srand(13);
	for(int i = 'a'; i <= 'z'; i++){
		while(keys.count(key) > 0) key = rand()%256;
		keys.insert(key);
		table[i] = key;
	}
	for(int i = 'A'; i <= 'Z'; i++){
		while(keys.count(key) > 0) key = rand()%256;
		keys.insert(key);
		table[i] = key;
	}
}

task get_task(){
	// Критическая секция. Задачи должны выдаваться последовательно
	pthread_mutex_lock(&mutex);

	task t;
	t.start = symbols_in_process;
	t.size = min((int)str.size() - symbols_in_process, task_size);
	symbols_in_process += t.size;

	pthread_mutex_unlock(&mutex);
	return t;
}

void* do_shifr(void *params){
	while(true){
		task t = get_task();
		// Условие остановки
		if(t.size == 0) break;

		for(int i = t.start; i < t.start + t.size; i++){
			int key = table[(int)str[i]];
			// Проверка является ли символ не буквой
			if(key == 0){
				*(bool*)params = false;
				return nullptr;
			}
			res[i] = (char)key;
		}
	}

	*(bool*)params = true;
	return nullptr;
}

int main () {
	gen_table();
	string path_in, path_out;

	cout << "Enter path to input file: ";
	cin >> path_in;
	cout << "Enter path to output file: ";
	cin >> path_out;

	ifstream input(path_in);
	ofstream output(path_out);

	if(!input.is_open() || !output.is_open()){
		cout << "File error";
		return 0;
	}

	input >> str;

	cout << "Enter threads_count: ";
	cin >> threads_count;
	if(threads_count <= 0){
		cout << "Invalid threads_count";
		return 0;
	}
	cout << "Enter task_size: ";
	cin >> task_size;
	if(task_size <= 0){
		cout << "Invalid task_size";
		return 0;
	}

	res = new char[str.size()];
	bool letter_only[threads_count];

	clock_t start_time = clock();
	pthread_mutex_init(&mutex, NULL) ;
	pthread_t* threads = new pthread_t[threads_count - 1];
	for(int i = 0; i < threads_count - 1; i++){
		pthread_create(&threads[i], NULL, do_shifr, (void *) &letter_only[i+1]);
	}
	do_shifr((void *) &letter_only[0]);

	for(int i = 0; i < threads_count - 1; i++){
		pthread_join(threads[i], NULL);
		letter_only[0] &= letter_only[i+1];
	}

	if(letter_only[0]){
		clock_t end_time = clock();
		cout << (end_time - start_time) << "ms";
		output << res;
	}else{
		cout << "Error: input string must contain only letters";
	}

	input.close();
	output.close();
}
