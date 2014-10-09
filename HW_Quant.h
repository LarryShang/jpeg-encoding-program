//-----Module definition for Quantize hardware IP block -------------
//-----You must modify this file as indicated by TODO comments-------
//-----You may reuse code from Programming Assignment 4 -------------
//-----as indicated by the REUSE comments----------------------- ----


#include "systemc.h"
#include "Block.h"
#include "Bus.h"
#include "AddressMap.h"

#define BLOCK_SIZE 64

#define INT_PULSE_WIDTH sc_time(1, SC_US)

//module
SC_MODULE(HW_Quant)
{
public:
	//constructor
	SC_CTOR(HW_Quant) {
		//instantiate the Quantize thread
		SC_THREAD(Quant_thread);
	}

	//define the Quantize thread method
	void Quant_thread();

	//REUSE: define the FIFO buffer interfaces
        //REUSE: name them InBuffer and OutBuffer
	sc_fifo_in<Block> InBuffer;
	sc_fifo_out<Block> OutBuffer;

};

// Read interface module
SC_MODULE(ReadIF)
{
public:
	// constructor
	SC_CTOR(ReadIF) {
		// instantiate the reader thread
		SC_THREAD(BusReader);
	}
	
	// define the bus reader method
	void BusReader();

	// TODO: define the FIFO output interface to hardware block
	// TODO: name it toQuantize
	sc_fifo_out<Block> toQuantize;

	// TODO: define the bus slave interface
	// TODO: name it bus_port
	sc_port<bus_slave_if> bus_port;
	// TODO: define the interrupt port for seeking data
	// TODO: name it int_input_seek
	sc_out<bool> int_input_seek;
}; //end read interface

SC_MODULE(WriteIF)
{
public:
        // constructor
        SC_CTOR(WriteIF) {
                // instantiate the write thread
                SC_THREAD(BusWriter);
        }

        // define the bus writer method
        void BusWriter();

	// TODO: define the FIFO input interface from hardware block
	// TODO: name it fromQuantize
	sc_fifo_in<Block> fromQuantize;
	// TODO: define the bus slave interface
	// TODO: name it bus_port
	sc_port<bus_slave_if> bus_port;

	// TODO: define the interrupt port for seeking data
	// TODO: name it int_output_avail
	sc_out<bool> int_output_avail;

}; //end read interface
 
// Hierarchical HW IP module
SC_MODULE(Quant_IP) 
{
public:
	// TODO: define the sub-modules
	WriteIF *writeif;
	ReadIF *readif;
	HW_Quant *hw_quant;

	// TODO: define the local channels between interface modules and HW_Quant;
	sc_fifo<Block> FIFO_DCT2Quant, FIFO_Quant2Zigzag;

	// TODO: define the top-level interrupt ports
	sc_out<bool> int_input_seek;
	sc_out<bool> int_output_avail;

	// TODO: define the bus slave interface
	sc_port<bus_slave_if> bus_slave;
	// constructor
	SC_CTOR(Quant_IP):FIFO_DCT2Quant(FIFO_SIZE), FIFO_Quant2Zigzag(FIFO_SIZE) // construct the local FIFOs in HW
                        {

                // TODO: instantiate the sub-modules
				hw_quant = new HW_Quant("HW_Quant");
				writeif = new WriteIF("WriteIF");
				readif = new ReadIF("ReadIF");

		// TODO: bind the FIFO ports
		readif -> toQuantize(FIFO_DCT2Quant);
		hw_quant -> InBuffer(FIFO_DCT2Quant);
		hw_quant -> OutBuffer(FIFO_Quant2Zigzag);
		writeif -> fromQuantize(FIFO_Quant2Zigzag);

		// TODO: bind the interrupt ports
		readif -> int_input_seek(int_input_seek);
		writeif -> int_output_avail(int_output_avail);

		// TODO: bind the bus ports
		readif -> bus_port(bus_slave);
		writeif -> bus_port(bus_slave);
	}// end constructor
}; // end Quantize IP definition
