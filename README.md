# Process-thread-synchronization
FIFO buffer with n elements (n>3). There is one producer and three consumers (A, B, C). 
Producer produces one element if there is a free space in the buffer. 
Element can be removed from the buffer when it is read by either consumer A and B or B and C. 
None of the consumers can read the same element multiple times. 
Consumer A cannot read the element if it was read before by consumer C and vice versa. 


