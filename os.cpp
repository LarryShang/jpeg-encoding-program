//-----Function definitions for classes os, os_taski and os_sem---
//-----REUSE this file as-is from Programming Assignment 4--------

#include "systemc.h"
#include "os.h"

using namespace std;

// implementation of os_task methods

//placeholder constructor since we have a pool
os_task::os_task() {
}

os_task::~os_task(){
}

void os_task::instantiate(sc_core::sc_process_handle handle, const char *name, unsigned int priority){
	static unsigned int count = 0;
	task_name = name;
	// All tasks are initially ready when created
	task_state = READY;
	task_id = count;
	task_priority = priority;
	task_handle = handle;
	activation_flag = false;
	count++;
}

const char *os_task::get_name(){
	return task_name.data();
}

enum state os_task::get_state(){
	return task_state;
}

unsigned int os_task::get_id(){
	return task_id;
}

unsigned int os_task::get_priority(){
	return task_priority;
}

sc_core::sc_process_handle os_task::get_handle(){
	return task_handle;
}

void os_task::activate(){
	// REUSE: set the activation flag and notify the event
	activation_flag = true;
	activation_event. notify();
	return;
}

void os_task::wait_for_activation(){
	// REUSE: If the activation flag is not set,
	// REUSE: wait for it to be set
		if(activation_flag == false)
			{
				wait(activation_event);
			}

        // REUSE: reset the activation flag

	// REUSE: update the task state
		activation_flag = false;
		task_state = READY;

	return;
}

//os class method implementation

os::os(){
	// empty constructor
}

os::~os(){

}

// dynamic task creation function
// this function is called by the main SC thread in the top module
// it may also be called by tasks to dynamically create children

os_task *os::task_create(
        const char* name,
        unsigned int priority,
	sc_core::sc_process_handle h)
{
        // get a task object from the pool 
        static int count = 0;
	os_task *t = &(task_pool[count++]);

	// REUSE: instantiate the user task in the OS model
	t ->instantiate(h, name, priority);
        // REUSE: add the task to the ALL_TASKS list
		ALL_TASKS.push_back(t);
        //newly created task is in the ready state
        // REUSE: add it to READY vector list
		READY_TASKS.push_back(t);
	// return the task pointer
        return t;
}

// task initialization function
// this function is called by the SystemC thread
// for the task, at the very beginning

void os::task_init(){

        // REUSE: find this task's pointer "t"
        os_task *t = NULL;
		for (int i = 0; i < ALL_TASKS.size(); i++){
		if(ALL_TASKS[i]->get_handle()== sc_get_current_process_handle())
			t=ALL_TASKS[i];
			}

        // REUSE: wait for the task to be scheduled by the OS
		t ->wait_for_activation();
        cout << sc_time_stamp() << ": Task " << t->get_name() << " is initialized" << endl;

	return;
}

// task termination function
// this function is called by the running task
// to terminate itself

void os::task_terminate(){
	// REUSE: update the state of the task
	os_task *t = NULL;
	t = (os_task*)CURRENT;
	t->task_state =  TERMINATED;
	// REUSE: do a rescheduling on remaining tasks
	// REUSE: wait till end of simulation if no tasks remain
	schedule();
	wait();

}

// this function displays the states of
// all the tasks in the order of their creation 
// It is called by the OS schedule method
// whenever a rescheduling takes place

void os::status(){
  cout << endl <<  sc_time_stamp() << ":" << endl;

  for (int i = 0; i < ALL_TASKS.size(); i++){
    os_task *t = ALL_TASKS[i];
    cout << t->get_name() << ": ";
    if (t->get_state() == RUNNING)
	cout << "RUNNING" << endl;
    else if (t->get_state() == READY)
	cout << "READY" << endl;
    else if (t->get_state() == SUSPENDED)
	cout << "SUSPENDED" << endl;
    else if (t->get_state() == TERMINATED)
	cout << "TERMINATED" << endl;
  }
}

void os::time_wait(sc_time t)
{
  

  // TODO: use simultaneous wait on time and preemption event
  // TODO: if preemption event arrives before all the time
  // TODO: has been consumed, go to ready state, wait for activation
  // TODO: then consume the remainder time
  // TODO: repeat until all of "t" has been consumed
  sc_time t_start, t_after, t_remain,t_consume, t_zero;
  t_zero = sc_time(0,SC_US);
  os_task *temp = NULL;
  temp= (os_task*)CURRENT;
  t_remain = t;
  t_consume = t_zero;
  while(t_remain > t_zero){
  	  t_start = sc_time_stamp();
  	  wait(t_remain,preemption);
	  t_after = sc_time_stamp();
	  t_consume = t_after - t_start + t_consume;
	  t_remain = t - t_consume;
	  if(t_remain > t_zero){
	  	temp->task_state =  READY;
		READY_TASKS.push_back(temp);
		schedule();
		temp->wait_for_activation();
	  	}
	  
  }

  return;
} // end time_wait

ERROR os::schedule() {

   // REUSE: If there are no tasks or all tasks have terminated
   // REUSE: Print the status and return code E_NO_TASKS
    

   // REUSE: If all tasks are suspended, just display
   // REUSE: the states and return code E_OK
   int count_t = 0;
   int count_s = 0;
   for (int i = 0; i < ALL_TASKS.size(); i++){
	 os_task *t = ALL_TASKS[i];
	 if(t->get_name()== NULL || t->get_state() == TERMINATED)
	 	count_t++;
	 if(t->get_state() == SUSPENDED)
	 	count_s++;
   	}
   if(count_t == ALL_TASKS.size()){
   	status();
    return E_NO_TASKS;
   	}
   if(count_s >0 && READY_TASKS.size()==0){
   	status();
   	return E_OK;
   	}
    

  // otherwise, we have some scheduling to do!
  // REUSE: find the highest priority ready task

  // REUSE: remove the highest priority task from the ready list
  // REUSE: and activate it!
	os_task *tmp = (os_task*)CURRENT;
     int tag = 0;
	if(READY_TASKS.size()!=0){
	  tmp = READY_TASKS[0];
	  for (int i = 0; i < READY_TASKS.size(); i++){
	  	if(tmp ->get_priority()< READY_TASKS[i] ->get_priority()){
		 	tmp = READY_TASKS[i];
			tag = i;
	  		}
  		}
	   CURRENT = tmp;
	   READY_TASKS.erase(READY_TASKS.begin()+tag);
	   CURRENT ->task_state = RUNNING;
	   CURRENT ->activate();
	}
	else{
	CURRENT = NULL;
	}
  // print out the status of all the tasks
  status();
  return E_OK;
} // end schedule

void os::run() {
  // all the initial tasks have been created
  // kickstart the scheduler
  cout << "Initializing Scheduler..." <<endl;
  schedule();
}

// os_sem method implementation
//-------------------------------------------------------------

int os_sem::wait() {
  int retval;
  class os_task *temp_t = OS->CURRENT;
  // TODO: if the semaphore value is 0
  // TODO: the accessing task must be blocked
  if (sc_semaphore::get_value()==0) {
          // TODO: block the caller
		  temp_t->task_state = SUSPENDED;
		  OS->CURRENT = NULL;

          cout << sc_time_stamp() <<": Task "<< temp_t->get_name() << " blocked on semaphore " << sc_semaphore::name() << "\n";

          // TODO: set the blocked_task pointer
          blocked_task = temp_t;

          // TODO: find the next active task and run it
          OS ->schedule();
		  blocked_task->wait_for_activation();
  }

  // proceed by running the base class method
  retval = sc_semaphore::wait();
  return retval;
} // end os_sem wait method


int os_sem::post() {
  int retval;
  // increment the semaphore value
  // by calling the base class method
  retval = sc_semaphore::post();
  // TODO: if there is a blocked task, it should be unblocked
  if (blocked_task != NULL) {
  	class os_task *temp_task = blocked_task;
          // TODO: if all tasks are suspended
          // TODO: run the scheduler here
          // TODO: after waking up the blocked task
          int suspend_count = 0;
		  int ready_count = 0;
          for (int i = 0; i < OS->ALL_TASKS.size(); i++){
			 os_task *t = OS->ALL_TASKS[i];
			 if(t->get_state() == SUSPENDED)
			 	suspend_count++;
			 if(t->get_state() == READY)
			 	ready_count++;
   			}
          if(ready_count == 0&& suspend_count > 0){
	          blocked_task ->task_state =READY;
	          OS->READY_TASKS.push_back(blocked_task);
			  OS ->schedule();
			}
          // TODO: if there is a running task with lower priority
          // TODO: than the newly unblocked task,
          // TODO: the running task should be preempted
          else if(OS->CURRENT!=NULL){
	          if(OS->CURRENT->get_priority() < blocked_task->get_priority()){
				  OS->CURRENT->task_state = READY;
				  temp_task ->task_state =READY;
				  OS->READY_TASKS.push_back(blocked_task);
				  blocked_task = NULL;
	          	
		  // TODO: the message below should be printed only in the case of preemption
	                  cout << sc_time_stamp() <<": Task "<< OS->CURRENT->get_name() << " preempted by task " << temp_task->get_name() << "\n";
				OS->preemption.notify();
				
			  }
			  else{
				temp_task ->task_state =READY;
				OS->READY_TASKS.push_back(blocked_task);
				blocked_task = NULL;
			  }
		  	}
  }

  return retval;
} // end os_sem post method

