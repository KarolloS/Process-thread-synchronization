//g++ -o mon mon.c -lpthread -lboost_thread

#include <boost/thread/thread.hpp>
#include <boost/random/mersenne_twister.hpp>

#include <iostream>
#include <queue>

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>

#include <sys/types.h> 
#include <sys/stat.h> 
#include <string.h> 
#include <errno.h> 
#include <fcntl.h> 
#include <pthread.h> 
#include <unistd.h>
#include <semaphore.h>

using namespace std;


#define NO_ONE      0
#define CONSUMER_A  1
#define CONSUMER_B  2
#define CONSUMER_C  3

#define READ        1
#define POP         2

// inicjalizacja generatora wartosci pseudolosowych
boost::random::mt19937 gen(time(0));

////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Semaphore
{
public:
  Semaphore( int value )
  {
     if( sem_init( & sem, 0, value ) != 0 )
       throw "sem_init: failed";
  }
  ~Semaphore()
  { 
	  sem_destroy( & sem ); 
  }

  void p()
  {
     if( sem_wait( & sem ) != 0 )
       throw "sem_wait: failed";
  }

  void v()
  {
     if( sem_post( & sem ) != 0 )
       throw "sem_post: failed";
  }

private:
	sem_t sem;
};


class Condition
{
  friend class Monitor;

public:
	Condition() : w( 0 )
	{
		waitingCount = 0;
	}

	void wait()
	{
		w.p();
	}

	bool signal()
	{
		if( waitingCount )
		{
			-- waitingCount;
			w.v();
			return true;
		}
		else
			return false;
	}

private:
	Semaphore w;
	int waitingCount; //liczba oczekujacych watkow
};


class Monitor
{
public:
	Monitor() : s( 1 ) {}

	void enter()
	{
		s.p();
	}

	void leave()
	{
		s.v();
	}

	void wait( Condition & cond )
	{
		++ cond.waitingCount;
		leave();
		cond.wait();
	}

	void signal( Condition & cond )
	{
		if( cond.signal() )
			enter();
	}

private:
	Semaphore s;
};

class buffer : public Monitor
{
public:
    buffer (int s) : who_read(NO_ONE), buffer_size(s), status(0) {}
    ~buffer () {}
    
    void produce()
    {
        enter();
        if(q.size() == buffer_size) wait(full);
        char i = 65 + gen() % (90 - 65);
		q.push_back(i);
		cout << "Producent odklada: " << i << ",         liczba elementow: " << q.size() << endl;
		print();
        if(q.size() > 0) signal(empty);
        if(q.size() > 3) signal(con_3);
        leave();
    }
    
    void read_or_pop(int consumer)
    {
        enter();
        
        switch(consumer)
        {
            case CONSUMER_A:
            if (who_read == CONSUMER_A || who_read == CONSUMER_C) wait(con_A);
            if (who_read == CONSUMER_B) 
            {
                status = POP;
            }
            else if (who_read == NO_ONE)
            {
                status = READ;
            }
            break;
            
            case CONSUMER_B:
            if (who_read == CONSUMER_B) wait(con_B);
            if (who_read == CONSUMER_A || who_read == CONSUMER_C) 
            {
                status = POP;
            }
            else if (who_read == NO_ONE)
            {
                status = READ;
            }
            break;
            
            case CONSUMER_C:
            if (who_read == CONSUMER_C || who_read == CONSUMER_A) wait(con_C);
            if (who_read == CONSUMER_B) 
            {
                status = POP;
            }
            else if (who_read == NO_ONE)
            {
                status = READ;
            }
            break;
        }
        
        if(q.size() == 0) wait(empty);
        
        if (status == POP)
        {
            if(q.size() <= 3) wait(con_3);
            char i = q.front();
			q.pop_front();
			cout << get_consumer(consumer) << " zdejmuje: " << i << ",      liczba elementow: " << q.size() << endl;
			print();
			who_read = NO_ONE;
        }
        else if (status == READ)
        {
            char i = q.front();
			cout << get_consumer(consumer) << " czyta: " << i << ",         liczba elementow: " << q.size() << endl;
			print();
			who_read = consumer;
        }
        
        if(q.size() < buffer_size) signal(full);
        
        if (who_read != CONSUMER_A && who_read != CONSUMER_C) signal(con_A);
        if (who_read != CONSUMER_B) signal(con_B);
        if (who_read != CONSUMER_C && who_read != CONSUMER_A) signal(con_C);
        
        leave();
    }
    
    string get_consumer(int c)
    {
        switch(c)
        {
            case CONSUMER_A:
            return "Konsument_A";
            break;
            
            case CONSUMER_B:
            return "Konsument_B";
            break;
            
            case CONSUMER_C:
            return "Konsument_C";
            break;
        }
    }
    
    void print() // funkcja drukuj¹ca zawartoœæ bufora
    {
        if (q.empty())
        {
            cout << "[]" << endl;
            return;
        }    
            
        cout << "  [ ";

        for (deque<char>::iterator it = q.begin(); it != q.end(); ++it)
        {
            cout << *it << " ";
        }
        cout << "]  " << endl;
    }
    
private:
    deque<char> q; // kolejka
    int who_read; // przechowuje informacje o tym kto przeczytal ostatni element
    
    int buffer_size;
    int status;
    
    Condition full, empty, con_A, con_B, con_C, con_3;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////

const int CONSUMER_A_MAX_SLEEP = 5; // w sekundach
const int CONSUMER_B_MAX_SLEEP = 6;
const int CONSUMER_C_MAX_SLEEP = 7;
const int PRODUCER_MAX_SLEEP = 3;

const int CONSUMER_A_MIN_SLEEP = 4;
const int CONSUMER_B_MIN_SLEEP = 3;
const int CONSUMER_C_MIN_SLEEP = 6;
const int PRODUCER_MIN_SLEEP = 1;

buffer buf(8); // utworzenie obiektu klasy bufor

inline void wait(int _min, int _max) // funkcja czekaj¹ca
{
	usleep(1000000 * _min + gen() % (1000000 * (_max - _min)));
}

void konsumentA()
{
	for (;;)
	{
		wait(CONSUMER_A_MIN_SLEEP, CONSUMER_A_MAX_SLEEP);
		cout << "Konsument A probuje zdjac...\n";
        buf.read_or_pop(CONSUMER_A);
	}
}

void konsumentC()
{
	for (;;)
	{
		wait(CONSUMER_C_MIN_SLEEP, CONSUMER_C_MAX_SLEEP);
		cout << "Konsument C probuje zdjac...\n";
        buf.read_or_pop(CONSUMER_C);
	}
}

void konsumentB()
{
	for (;;)
	{
		wait(CONSUMER_B_MIN_SLEEP, CONSUMER_B_MAX_SLEEP);
		cout << "Konsument B probuje zdjac...\n";
        buf.read_or_pop(CONSUMER_B);
	}
}

void producent()
{
	for (;;)
	{
		wait(PRODUCER_MIN_SLEEP, PRODUCER_MAX_SLEEP);
		cout << "Producent probuje odlozyc...\n";
        buf.produce();

	}
}

int main()
{
	boost::thread thread1(konsumentA);
	boost::thread thread2(konsumentB);
	boost::thread thread3(konsumentC);
	boost::thread thread4(producent);
	thread1.join();
	thread2.join();
	thread3.join();
	thread4.join();

	return 0;
}
