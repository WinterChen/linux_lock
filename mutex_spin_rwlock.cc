#include<sys/types.h> 
#include<sys/socket.h> 
#include<unistd.h> 
#include<stdio.h> 
#include<stdlib.h> 
#include<errno.h> 
#include<string.h>
#include<iostream>
#include<pthread.h>
#include<string>
#include<vector>
#include<sys/syscall.h>   

#include <urcu-qsbr.h>

using namespace std;
struct Foo{
	Foo():a(0), b(0),c(0),d(0){}
	int a;
	int b;
	int c;
	int d; 
};
Foo *gs_foo = new Foo;
int g_read_loop_count  = 10000;
int g_write_loop_count  = 10000;

/*****************************mutex*********************/
pthread_mutex_t   gMutex;
void* ReadThreadFuncMutex(void* arg) {
    struct Foo* foo = NULL;
    int sum = 0;
    
    for (int i = 0; i < g_read_loop_count; ++i) {
        
            pthread_mutex_lock(&gMutex);
            foo = gs_foo;
            if (foo) {
                sum += foo->a + foo->b + foo->c + foo->d;
            }
            pthread_mutex_unlock(&gMutex);
       
       
    }
	cout << "read end" << endl;
}
void* WriteThreadFuncMutex(void* arg) {
    for (int i = 0; i < g_write_loop_count; ++i) {
       	pthread_mutex_lock(&gMutex);
        gs_foo->a = 2; gs_foo->b = 3; 
        gs_foo->c = 4; gs_foo->d = 5;
        pthread_mutex_unlock(&gMutex);
    } 
	cout << "write end" << endl;
}

/*****************************spinlock*********************/
pthread_spinlock_t  gSpinlock;
void* ReadThreadFuncSpin(void* arg) {
    struct Foo* foo = NULL;
    int sum = 0;
    
    for (int i = 0; i < g_read_loop_count; ++i) {
        
            pthread_spin_lock(&gSpinlock);
            foo = gs_foo;
            if (foo) {
                sum += foo->a + foo->b + foo->c + foo->d;
            }
            pthread_spin_unlock(&gSpinlock);
        
       
    }
	cout << "read end" << endl;
}

void* WriteThreadFuncSpin(void* arg) {
    for (int i = 0; i < g_write_loop_count; ++i) {
       	pthread_spin_lock(&gSpinlock);
        gs_foo->a = 2; gs_foo->b = 3; 
        gs_foo->c = 4; gs_foo->d = 5;
        pthread_spin_unlock(&gSpinlock);
    } 
	cout << "write end" << endl;
}


/*****************************rwlock*********************/
pthread_rwlock_t   gRwlock;
void* ReadThreadFuncRW(void* arg) {
    struct Foo* foo = NULL;
    int sum = 0;
    
    for (int i = 0; i < g_read_loop_count; ++i) {
        
            pthread_rwlock_rdlock(&gRwlock);
            foo = gs_foo;
            if (foo) {
                sum += foo->a + foo->b + foo->c + foo->d;
            }
            pthread_rwlock_unlock(&gRwlock);
       
       
    }
	cout << "read end" << endl;
}

void* WriteThreadFuncRW(void* arg) {
    for (int i = 0; i < g_write_loop_count; ++i) {
       	pthread_rwlock_wrlock(&gRwlock);
        gs_foo->a = 2; gs_foo->b = 3; 
        gs_foo->c = 4; gs_foo->d = 5;
        pthread_rwlock_unlock(&gRwlock);
    } 
	cout << "write end" << endl;
}

int main(int argc, char **argv){
	
	if (argc < 5) {
		cout << "Usage: " << argv[0] << " <read_thread_count> <write_thread_count> <read_loop_count> <write_loop_count>"<< endl;
		exit(0);
	}
	int read_thread_count = atoi(argv[1]);
	int write_thread_count = atoi(argv[2]);
	g_read_loop_count = atoi(argv[3]);
	g_write_loop_count = atoi(argv[4]);
	
	cout << "read_thread_cout: " << read_thread_count << " write_thread_cout: " << write_thread_count << endl;
	cout << "please select which test:  1 mutex, 2 spinlock, 3 rwlock" << endl;
	int selected = 1;
	cin >> selected;
	typedef void* (*ThreadFunc)(void* arg);
	ThreadFunc rf, wf;
	int res;
	switch (selected){
		case 1:
			res = pthread_mutex_init(&gMutex, NULL);
			if(res != 0){
				cout << "init mutex err: " << res << endl;
				return 0;
			}
			rf = ReadThreadFuncMutex;
			wf = WriteThreadFuncMutex;
			break;
		case 2:
			res = pthread_spin_init(&gSpinlock, 0);
			if(res != 0){
				cout << "init mutex err: " << res << endl;
				return 0;
			}
			rf = ReadThreadFuncSpin;
			wf = WriteThreadFuncSpin;
			break;
		case 3:
			res = pthread_rwlock_init(&gRwlock, NULL);
			if(res != 0){
				cout << "init mutex err: " << res << endl;
				return 0;
			}
			rf = ReadThreadFuncRW;
			wf = WriteThreadFuncRW;
			break;
		default:
			cout << "input error, exit" << endl;
			return 1;
	}
	vector<pthread_t> threadFdVec;
	for(int i = 0; i < read_thread_count; i++){
		pthread_t thread_fd; 
		pthread_create(&thread_fd, NULL, rf, NULL);
		threadFdVec.push_back(thread_fd);
	}
	for(int i = 0; i < write_thread_count; i++){
		pthread_t thread_fd; 
		pthread_create(&thread_fd, NULL, wf, NULL);
		threadFdVec.push_back(thread_fd);
	}
	vector<pthread_t>::iterator iter = threadFdVec.begin();
	while(iter != threadFdVec.end()){
		int res = pthread_join(*iter, NULL);
		cout << "thread " << *iter  << " stopped, " << res << endl;
		iter++;
	}

	return 0;
}
