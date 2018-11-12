#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>

using namespace std;
#define MemSize 256 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct {
    bitset<32>  PC;
    bool        nop;  
};

struct IDStruct {
    bitset<32>  Instr;
    bool        nop;  
};

struct EXStruct {
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<16>  Imm;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        is_I_type;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu 
    bool        wrt_enable;
    bool        nop;  
};

struct MEMStruct {
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;    
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;    
    bool        nop;    
};

struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;     
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;     
};

struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};

class RF
{
    public: 
        bitset<32> Reg_data;
     	RF()
    	{ 
			Registers.resize(32);  
			Registers[0] = bitset<32> (0);  
        }
	
        bitset<32> readRF(bitset<5> Reg_addr)
        {   
            Reg_data = Registers[Reg_addr.to_ulong()];
            return Reg_data;
        }
    
        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
        {
            Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
        }
		 
		void outputRF()
		{
			ofstream rfout;
			rfout.open("RFresult.txt",std::ios_base::app);
			if (rfout.is_open())
			{
				rfout<<"State of RF:\t"<<endl;
				for (int j = 0; j<32; j++)
				{        
					rfout << Registers[j]<<endl;
				}
			}
			else cout<<"Unable to open file";
			rfout.close();               
		} 
			
	private:
		vector<bitset<32> >Registers;	
};

class INSMem
{
	public:
        bitset<32> Instruction;
        INSMem()
        {       
			IMem.resize(MemSize); 
            ifstream imem;
			string line;
			int i=0;
			imem.open("imem.txt");
			if (imem.is_open())
			{
				while (getline(imem,line))
				{      
					IMem[i] = bitset<8>(line);
					i++;
				}                    
			}
            else cout<<"Unable to open file";
			imem.close();                     
		}
                  
		bitset<32> readInstr(bitset<32> ReadAddress) 
		{    
			string insmem;
			insmem.append(IMem[ReadAddress.to_ulong()].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+1].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+2].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+3].to_string());
			Instruction = bitset<32>(insmem);		//read instruction memory
			return Instruction;     
		}     
      
    private:
        vector<bitset<8> > IMem;     
};
      
class DataMem    
{
    public:
        bitset<32> ReadData;  
        DataMem()
        {
            DMem.resize(MemSize); 
            ifstream dmem;
            string line;
            int i=0;
            dmem.open("dmem.txt");
            if (dmem.is_open())
            {
                while (getline(dmem,line))
                {      
                    DMem[i] = bitset<8>(line);
                    i++;
                }
            }
            else cout<<"Unable to open file";
                dmem.close();          
        }
		
        bitset<32> readDataMem(bitset<32> Address)
        {	
			string datamem;
            datamem.append(DMem[Address.to_ulong()].to_string());
            datamem.append(DMem[Address.to_ulong()+1].to_string());
            datamem.append(DMem[Address.to_ulong()+2].to_string());
            datamem.append(DMem[Address.to_ulong()+3].to_string());
            ReadData = bitset<32>(datamem);		//read data memory
            return ReadData;               
		}
            
        void writeDataMem(bitset<32> Address, bitset<32> WriteData)            
        {
            DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0,8));
            DMem[Address.to_ulong()+1] = bitset<8>(WriteData.to_string().substr(8,8));
            DMem[Address.to_ulong()+2] = bitset<8>(WriteData.to_string().substr(16,8));
            DMem[Address.to_ulong()+3] = bitset<8>(WriteData.to_string().substr(24,8));  
        }   
                     
        void outputDataMem()
        {
            ofstream dmemout;
            dmemout.open("dmemresult.txt");
            if (dmemout.is_open())
            {
                for (int j = 0; j< 1000; j++)
                {     
                    dmemout << DMem[j]<<endl;
                }
                     
            }
            else cout<<"Unable to open file";
            dmemout.close();               
        }             
      
    private:
		vector<bitset<8> > DMem;      
};  

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate<<"State after executing cycle:\t"<<cycle<<endl; 
        
        printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;        
        printstate<<"IF.nop:\t"<<state.IF.nop<<endl; 
        
        printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl; 
        printstate<<"ID.nop:\t"<<state.ID.nop<<endl;
        
        printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
        printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
        printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl; 
        printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
        printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
        printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
        printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl; 
        printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
        printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;        
        printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
        printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
        printstate<<"EX.nop:\t"<<state.EX.nop<<endl;        

        printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
        printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl; 
        printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
        printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;   
        printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;              
        printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
        printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl; 
        printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;         
        printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;        

        printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
        printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
        printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;        
        printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
        printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;        
        printstate<<"WB.nop:\t"<<state.WB.nop<<endl; 
    }
    else cout<<"Unable to open file";
    printstate.close();
}
 
unsigned long shiftbits(bitset<32> inst, int start)
{
	unsigned long ulonginst;
	return ((inst.to_ulong()) >> start);
}

bitset<32> signextend(bitset<16> imm)
{
	string sestring;
	if (imm[15] == 0) {
		sestring = "0000000000000000" + imm.to_string<char, std::string::traits_type, std::string::allocator_type>();
	}
	else {
		sestring = "1111111111111111" + imm.to_string<char, std::string::traits_type, std::string::allocator_type>();
	}
	return (bitset<32>(sestring));
}


int main()
{
    
    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;
	stateStruct state, newState;
	int cycle = 0;			//track cycle
	int haltcycle = -2;		//whcih cycle we get first halt instr
	bitset<32> signext;		//SignExtImm

	//nop initialization
	state.IF.nop = 0;
	state.ID.nop = 1;
	state.EX.nop = 1;
	state.MEM.nop = 1;
	state.WB.nop = 1;
	newState.IF.nop = 0;
	newState.ID.nop = 1;
	newState.EX.nop = 1;
	newState.MEM.nop = 1;
	newState.WB.nop = 1;
	//pc initialization
	state.IF.PC = 0;
    
	//initialization
	
	state.EX.is_I_type = 0;
	state.EX.rd_mem = 0;
	state.EX.wrt_mem = 0;
	state.EX.wrt_enable = 0;
	state.EX.alu_op = 1;
	state.MEM.rd_mem = 0;
	state.MEM.wrt_enable = 0;
	state.MEM.wrt_mem = 0;
	state.WB.wrt_enable = 0;

	newState.EX.is_I_type = 0;
	newState.EX.rd_mem = 0;
	newState.EX.wrt_mem = 0;
	newState.EX.wrt_enable = 0;
	newState.EX.alu_op = 1;
	newState.MEM.rd_mem = 0;
	newState.MEM.wrt_enable = 0;
	newState.MEM.wrt_mem = 0;
	newState.WB.wrt_enable = 0;
	

    while (1) {

        /* --------------------- WB stage --------------------- */

		if (state.WB.nop == 0)
		{
			//check whether halt, set nop
			if (cycle == haltcycle + 3)
				newState.WB.nop = 1;

			if(state.WB.wrt_enable)
				myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);

		}


        /* --------------------- MEM stage --------------------- */

		if (state.MEM.nop == 0)
		{
			newState.WB.nop = 0;
			//check whether halt, set nop
			if (cycle == haltcycle + 2)
				newState.MEM.nop = 1;

			//read write MEM
			if (state.MEM.wrt_mem)
				continue;
			//	myDataMem.writeDataMem(state.MEM.ALUresult, myRF.readRF(state.MEM.Rt));
			else if (state.MEM.rd_mem)
				continue;
				//newState.WB.Wrt_data = myDataMem.readDataMem(state.MEM.ALUresult);
			else
				newState.WB.Wrt_data = state.MEM.ALUresult;

			//pass ff values to WB
			newState.WB.Rs = state.MEM.Rs;
			newState.WB.Rt = state.MEM.Rt;
			newState.WB.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
			newState.WB.wrt_enable = state.MEM.wrt_enable;


		}

        /* --------------------- EX stage --------------------- */

		if (state.EX.nop == 0)
		{
			newState.MEM.nop = 0;
			//check whether halt, set nop
			if (cycle == haltcycle + 1)
				newState.EX.nop = 1;

			//calculate result
			newState.MEM.ALUresult =
				(state.EX.alu_op == 1) ? bitset<32>(state.EX.Read_data1.to_ulong() + state.EX.Read_data2.to_ulong()) : bitset<32>(state.EX.Read_data1.to_ulong() - state.EX.Read_data2.to_ulong());

			//pass ff values to MEM
			newState.MEM.Store_data = state.EX.Read_data2;
			newState.MEM.Rs = state.EX.Rs;
			newState.MEM.Rt = state.EX.Rt;
			newState.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
			newState.MEM.wrt_enable = state.EX.wrt_enable;
			newState.MEM.rd_mem = state.EX.rd_mem;
			newState.MEM.wrt_mem = state.EX.wrt_mem;


		}
          

        /* --------------------- ID stage --------------------- */

		if (state.ID.nop == 0)
		{
			newState.EX.nop = 0;
			bitset<6> opcode = bitset<6>(shiftbits(state.ID.Instr, 26));
			bitset<6> funct = bitset<6>(shiftbits(state.ID.Instr, 0));

			//control signals
			newState.EX.is_I_type = (opcode.to_ulong() == 0) ? 0 : 1;
			//IsBranch = (opcode.to_ulong() == 4) ? 1 : 0;

			newState.EX.alu_op = (funct.to_ulong() == 35) ? 0 : 1;

			newState.EX.rd_mem = (opcode.to_ulong() == 35) ? 1 : 0;
			newState.EX.wrt_mem = (opcode.to_ulong() == 43) ? 1 : 0;

			newState.EX.wrt_enable = (opcode.to_ulong() == 35) ? 1 : 0;

			//RF addresses
			newState.EX.Rs = bitset<5>(shiftbits(state.ID.Instr, 21));
			newState.EX.Rt = bitset<5>(shiftbits(state.ID.Instr, 16));
			newState.EX.Imm = bitset<16>(shiftbits(state.ID.Instr, 0));
			signext = signextend(newState.EX.Imm);

			newState.EX.Wrt_reg_addr = (newState.EX.is_I_type) ? newState.EX.Rt : bitset<5>(shiftbits(state.ID.Instr, 11));

			//read RF
			newState.EX.Read_data1 = myRF.readRF(newState.EX.Rs);
			newState.EX.Read_data2 = myRF.readRF(newState.EX.Rt);
			
		}
	

        
        /* --------------------- IF stage --------------------- */

		
		if (state.IF.nop == 0)
		{
			newState.IF.nop = 0;
			newState.ID.nop = 0;
			//fetch instr, next pc+=4
			newState.ID.Instr = myInsMem.readInstr(state.IF.PC);
			newState.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4);

			//check whether halt, set nop
			if (newState.ID.Instr.to_string<char, std::string::traits_type, std::string::allocator_type>() == "11111111111111111111111111111111"&&haltcycle == -2)
			{
				haltcycle = cycle;
				newState.IF.nop = 1;
				newState.ID.nop = 1;
			}
			
			/*braddr = bitset<32>(
				pc.to_ulong() + 4
				+ (bitset<32>((bitset<30>(shiftbits(signext, 0))).to_string<char, std::string::traits_type, std::string::allocator_type>() + "00")).to_ulong());

			pc = (IsBranch.to_ulong() && IsEq.to_ulong()) ? braddr : ((JType.to_ulong()) ? jaddr : pcplusfour);*/

		
		}






             
        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
            break;
        
        printState(state, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 
       
        state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */ 
		cycle += 1;
                	
    }
    
    myRF.outputRF(); // dump RF;	
	myDataMem.outputDataMem(); // dump data mem 
	
	return 0;
}