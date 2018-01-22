# Process-thread-synchronization

FIFO buffer with n elements (n>3). There is one producer and three consumers (A, B, C). 
Producer produces one element if there is a free space in the buffer. 
Element can be removed from the buffer when it is read by either consumer A and B or B and C. 
None of the consumers can read the same element multiple times. 
Consumer A cannot read the element if it was read before by consumer C and vice versa. 

Problem solved using two different approaches: 
* semaphores `sem.c` - each consumer and producer is a different process and use shared memory. More about [shared memory](http://man7.org/linux/man-pages/man2/mmap.2.html) and [semaphores](http://pubs.opengroup.org/onlinepubs/7908799/xsh/semaphore.h.html). 
* monitors `mon.cpp` - each consumer and producer is a different thread and use process memory. I'm using my own implementation of monitors. More about [boost thread library](http://www.boost.org/doc/libs/1_64_0/doc/html/thread.html).

Project was done as a part of Operating Systems Course at Warsaw University of Technology.
