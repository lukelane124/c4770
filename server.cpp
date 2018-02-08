#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <pthread.h>


//using namespace std;

long long sum = 0;

void* sum_runner(void* arg) {//Thread funciton to generate sum of zero to N.
  long long* limit_ptr = (long long*) arg;
  long long limit = *limit_ptr;

  for (long long i = 0; i <= limit; i++) {
    sum += i;
  }

  // TODO: What to do with the answer?

  // Sum is a global variable, so other threads can access.
  
}
int main(int argc, char** argv) {
  if (argc < 2) {
    printf("Usage: %s <num>\n", argv[0]);
    exit(-1);
  }

  long long limit = atoll(argv[1]);

  //Thread id
  pthread_t tid;

  // Create attibutes
  pthread_attr_t attr;
  pthread_attr_init(&attr);

  pthread_create(&tid, &attr, sum_runner, &limit);
  


  //Wait until thread has done its work.
  pthread_join(tid, NULL);
  
  printf("Sum is %lld\n", sum);  

}
