//---------Defines the CPU module---------------
//---------You must edit this file--------------
//---------based on the TODO comments-----------
//---------REUSE the code from Programming Assignment 4 as noted-----------

#include "systemc.h"
#include "Block.h"
#include "os.h"
#include "Bus.h"
#include "AddressMap.h"

// define the task priorities 

#ifndef READ_PRIORITY
#define READ_PRIORITY 1
#endif

#ifndef DCT_PRIORITY
#define DCT_PRIORITY 1
#endif

#ifndef ZIGZAG_PRIORITY
#define ZIGZAG_PRIORITY 1
#endif

#ifndef HUFFMAN_PRIORITY
#define HUFFMAN_PRIORITY 1
#endif


//module
SC_MODULE(CPU)
{
	public:
	//constructor
	SC_CTOR(CPU) {
                // REUSE: Instantiate the OS and the os_fifo channels here
                // note that write delay is 6 MS and read delay is 4 MS
                // sizes of the FIFO channels is given by macro FIFO_SIZE
		OS = new os();
		Read2DCT = new typename os_fifo<Block>::os_fifo(OS,"Read2DCT",FIFO_SIZE,sc_time(6,SC_MS),sc_time(4,SC_MS));
		Zigzag2Huff = new typename os_fifo<Block>::os_fifo(OS,"Zigzag2Huff",FIFO_SIZE,sc_time(6,SC_MS),sc_time(4,SC_MS));
		write_to_Quantize = new os_sem(OS,0);
		read_from_Quantize = new os_sem(OS,0);
		BusAccess = new os_sem(OS,1);
		//define the top level thread
		SC_THREAD(top);

		// REUSE: register the ISRs using SC_METHOD
		// REUSE: assign the sensitivity list to trigger on interrupt
		// REUSE: but make sure you don't run the ISRs at model initialization
                SC_METHOD(ISR_Quantize_input_seek);
				sensitive <<   int_Quantize_input_seek.pos();
				dont_initialize();
				SC_METHOD(ISR_Quantize_output_avail);
				sensitive <<   int_Quantize_output_avail.pos();
				dont_initialize();
	}

	// CPU software definitions
	//------------------------------------------------------------------
	// define the thread functions here
	// but the actual threads will be spawned dynamically
	void top();
	void Read_thread();
	void DCT_thread();
	void Zigzag_thread();
	void Huff_thread();

	// define bus write and read functions for the respective FIFOs
	void BusRead_Quantize2Zigzag(Block *inBlock);
	void BusWrite_DCT2Quantize(Block *outBlock);

	// define the bus access semaphore
	// a mutex would be better, but we can reuse os_sem
	os_sem *BusAccess;

	// define the ISR methods
	// these are defined statically
	void ISR_Quantize_input_seek();
	void ISR_Quantize_output_avail();

	//define the FIFO pointers
	os_fifo<Block> *Read2DCT;
	os_fifo<Block> *Zigzag2Huff;

	// define the semaphore pointers
	os_sem *write_to_Quantize, *read_from_Quantize;

	// pointer to the operating system model
	os *OS;

	// CPU hardware port definitions
	//------------------------------------------------------------------
	// TODO: Define the CPU->Bus port, name it bus_port
	sc_port<bus_master_if> bus_port;

	// REUSE: deine the interrupt ports
        sc_in<bool> int_Quantize_input_seek;
		sc_in<bool> int_Quantize_output_avail;
};
