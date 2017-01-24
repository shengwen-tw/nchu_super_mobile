#ifndef _QUEUE_H_
#define _QUEUE_H_

#define QUEUE_SIZE 10

class Queue{
  private:
  float queue[QUEUE_SIZE] = {0.0};
  int queue_size = 0;

  public:
  void push(float new_data);
  int pop(float *data);
  int size();
};

#endif
