//-----Definition of the hierarchical system module---------------
//-----You must modify this file as indicated by TODO comments----
//-----You may reuse code from Programming Assignment 4-----------
//-----as indicated by REUSE comments-----------------------------

#include "systemc.h"
#include "Block.h"
#include "CPU.h"
#include "HW_Quant.h"

//top-level module
SC_MODULE(System)
{
private:
	// define the SW-HW modules
	CPU *cpu;
	Quant_IP *hw_quant;

	// TODO: define the bus channel
	bus_channel Bus;
	// instantiation of the interrupts
	sc_signal<bool> interrupt_Quant_input_seek, interrupt_Quant_output_avail;

public:
	//constructor
	SC_CTOR(System):Bus("Bus") // TODO: delegated constructor for bus channel (name it "Bus")
		{

		// RESUE: instantiate the system-level modules
		// REUSE: name them "CPU" and "Quantize_block"
		cpu = new CPU("CPU");
		hw_quant= new Quant_IP("Quantize_block");

		// TODO: bind the bus ports
		cpu -> bus_port(Bus);
		hw_quant -> bus_slave(Bus);

		// REUSE: bind the interrupt ports
		cpu ->int_Quantize_input_seek.bind(interrupt_Quant_input_seek);
		cpu ->int_Quantize_output_avail.bind(interrupt_Quant_output_avail);
		hw_quant ->int_input_seek.bind(interrupt_Quant_input_seek);
		hw_quant ->int_output_avail.bind(interrupt_Quant_output_avail);
	}
};

