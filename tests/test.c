#include<stdio.h>
#include<stdlib.h>

int main(int argc, char** argv){

  int arr[100];
  int num, i;

  size_t result = 0;

  for(i = 0; i < 100; i++){
    arr[i] = rand();
  }

  printf("enter a number\n");
  scanf("%d", &num);

  
  for(i = 0; i< num; i++){
    result = result + arr[i];
  }
 
  printf("the result is %zd\n", result);

  return 0;
}
