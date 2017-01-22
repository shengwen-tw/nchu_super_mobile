#include "queue.hpp"

float queue[QUEUE_SIZE] = {0.0};
int queue_size = 0;

void push(float new_data)
{
  if(queue_size < QUEUE_SIZE) {
    queue_size++;
  }

  for(int i = queue_size - 1; i > 0; i--) {
    queue[i] = queue[i - 1];
  }

  queue[0] = new_data;
}

int pop(float *data)
{
  if(queue_size == 0) {
    return 1;
  }

  *data = queue[queue_size - 1];
  queue[queue_size - 1] = 0;
  queue_size--;

  return 0;
}
