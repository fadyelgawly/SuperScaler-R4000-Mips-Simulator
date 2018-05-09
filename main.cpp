#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <fstream>
using namespace std;

uint32_t    regs[15], memory[32];
struct BranchPredictor
{
	uint32_t pc;
	uint32_t targetaddress;
	int state;      //state 0: Stronglt NT --state 1: NT -- State 2: Taken -- State 3: Strongly Taken
};

struct instWord
{
	string instText;
	unsigned int instMachineCode = 0;
	signed int rd = 0, rs1 = 0, rs2 = 0, shamt = 0, funct = 0, opcode = 0, alucontrol = 0;
	signed int I_imm = 0, S_imm = 0, B_imm = 0, U_imm = 0, J_imm = 0;
	unsigned pc = 0;
	int zeroflag = 0, alures = 0, datamemoryresult = 0;
	uint32_t s1data = 0, s2data = 0;
	vector <BranchPredictor> branch;
	bool regwrite = 0,
		ALUSrc = 0,
		memtoreg = 0,
		regdst = 0,
		Branch = 0,
		memorywrite = 0,
		jump = 0,
		Branchequal = 0;

};
void parseb(instWord &W);
void Registersreset();                              //initialize all registers to 0
void RegisterFile(instWord &A);
void ALU(instWord &A);
void datamemoryreset();
void datamemory(instWord &A);
void checkpc(uint32_t add, uint32_t target, instWord A);
vector <instWord> ROM;
void updatestate(uint32_t pc, uint32_t targetadd, bool currentstate, instWord A);
int get_state(uint32_t address, instWord A);
uint32_t get_target_address(uint32_t pc1, instWord A);

void decoder(instWord & instruction);
void  control_unit(instWord & A);
bool fowardingunit(instWord &W)
{
	if ((W.rd == ROM[W.pc + 1].rs1) || (W.rd == ROM[W.pc + 1].rs2)) {
		return true;
	}



	return 0;
}
void printRegs();
void signExtend(instWord & W) { if (W.I_imm >> 15) W.I_imm += 0xFFFF0000; }


void IF(instWord &W) {
	cout << "IF:" << W.instText << "\t";
	decoder(W);

}     // –first half of fetching of instruction; PC selection happens here as well as initiation of instruction cache access
void IS(instWord &W) {
	cout << "IS:" << W.instText << "\t";
	signExtend(W);
	// Branch predictor
}    // –second half of access to instruction cache.
void RF(instWord &W) {
	cout << "RF:" << W.instText << "\t";
	RegisterFile(W);
	control_unit(W);
}  // –instruction decode and register fetch, hazard checking and also instruction cache hit detection.
void EX(instWord &W) {
	cout << "EX:" << W.instText << "\t";
	ALU(W);
}    //–execution, which includes effective address calculation, ALU operation, and branch target computation and condition evaluation. –
void DF(instWord &W) {
	cout << "DF:" << W.instText << "\t";
	datamemory(W);
}    //–data fetch, first half of access to data cache.
void DS(instWord &W) { cout << "DS:" << W.instText << "\t"; }    //–second half of access to data cache. –
void TC(instWord &W) { cout << "TC:" << W.instText << "\t"; }   //–tag check, determine whether the data cache access hit. –
void WB(instWord &W) {
	cout << "WB:" << W.instText << "\t";
	RegisterFile(W);
}  //–write back for loads and register-register operations.

bool stall(vector <instWord> W)   //gets the decision whether to stall or continue
{
	if (W[5].memtoreg)
	{
		if (W[5].rs2 != 0)
		{
			if ((W[3].rs1 == W[4].rs2) || (W[3].rs2 == W[4].rs2))
			{
				return true;
			}
		}
	}

	if (W[6].regwrite)
	{
		if (W[6].rs2 != 0)
		{
			if ((W[4].rd == W[3].rs1) || (W[6].rd == W[3].rs2))
				return true;
		}
	}
	return false;
}


int main(int argc, char *argv[]) {
	Registersreset();
	vector <instWord> W;                //ROM
	instWord testVariable;              //Variable to fill the rom
	testVariable.instText = "   ";
	for (int i = 0; i < 7; i++) W.push_back(testVariable);  //INIT ROM WITH 8 EMPTY Instructions, Simulates initializing of buffers

	testVariable.instMachineCode = 0x20020001;  testVariable.instText = "Addi";        //TAKEN FROM SLIDES
	W.push_back(testVariable);                          //SHOULD CAUSE STALLING
	testVariable.instMachineCode = 0xac020000; testVariable.instText = "Sw";
	W.push_back(testVariable);
	testVariable.instMachineCode = 0x8c030000; testVariable.instText = "Lw";
	W.push_back(testVariable);
	testVariable.instMachineCode = 0x631820; testVariable.instText = "Add";
	W.push_back(testVariable);
	testVariable.instMachineCode = 0; testVariable.instText = "   ";
	for (int i = 0; i < 7; i++) W.push_back(testVariable);  //Should

	int j = 0;
	bool Stall = false;

	while (W.size() - 7 != 0) {
		//IF STAGE
		do {
			IF(W[7]);
			Stall = stall(W);
		} while (Stall);

		//IS STAGE
		do {
			IS(W[6]);
			Stall = stall(W);
		} while (Stall);
		do {
			RF(W[5]);
			Stall = stall(W);
		} while (Stall);
		do {
			EX(W[4]);
			Stall = stall(W);
		} while (Stall);

		do {
			DF(W[3]);
			Stall = stall(W);
		}

		while (Stall);
		do {
			DS(W[2]);
			Stall = stall(W);
		}

		while (Stall);

		do {
			TC(W[1]);
			Stall = stall(W);
		}

		while (Stall);

		do {
			WB(W[0]);
			Stall = stall(W);
		} while (Stall);
		cout << endl;
		for (int i = 0; i <= j; i++) cout << "\t\t";
		j++;
		W.erase(W.begin());
		W.shrink_to_fit();


	}
	cout << endl;

	printRegs();
	return 0;

}

void parseb(instWord &W) {

	string temp = W.instText.substr(0, W.instText.find("\t"));

	if (temp != "main:\r") {
		if (temp == "add") {            //R Type
			W.opcode = 0x0;
			W.shamt = 0;
			W.funct = 0x0;
			W.instText.erase(0, 4);
			W.rd = stoi(W.instText.substr(2, W.instText.find(",") - 1));
			W.instText.erase(0, W.instText.find(",") + 2);
			W.rs1 = stoi(W.instText.substr(1, W.instText.find(",")));
			W.instText.erase(0, W.instText.find(",") + 2);
			W.rs2 = stoi(W.instText.substr(1, W.instText.find("\t")));

			W.instMachineCode = W.opcode << 26;
			W.instMachineCode |= W.rs2 << 21;
			W.instMachineCode |= W.rs1 << 16;
			W.instMachineCode |= W.rd << 11;
			W.instMachineCode |= W.shamt << 6;
			W.instMachineCode |= W.funct;
		}

	}
}
void Registersreset()                              //initialize all registers to 0
{
	for (int i = 0; i < 15; i++)
		regs[i] = 0;

}
void RegisterFile(instWord &A)    //Read and Write from registers
{

	if ((A.regwrite) && (A.rd<16) && ((A.opcode == 0x8) || (A.opcode == 0x23)))
	{
		regs[A.rs2] = A.alures;
		//Write
	}
	else
		if ((A.regwrite) && (A.rd<16) && ((A.opcode == 0x20) || (A.opcode == 0x0)))
		{
			regs[A.rd] = A.alures;
		}

	if (!(A.regwrite) && (A.rs1<16) && (A.rs2<16))
	{
		A.s1data = regs[A.rs1];                             //Read
		A.s2data = regs[A.rs2];
	}
}

void ALU(instWord &A)
{
	int aluresult = 0;
	if (A.ALUSrc) {
		switch (A.alucontrol) {
		case 1: A.alures = A.s1data + A.I_imm; break;                 //ADD
		case 2: A.alures = A.s1data - A.I_imm; break;                 //SUB
		case 3: A.alures = A.s1data & A.I_imm; break;                 //AND
		case 4: A.alures = A.s1data || A.I_imm; break;                //OR
		case 5: A.alures = A.s1data ^ A.I_imm; break;                //XOR

		case 6: A.alures = signed(A.I_imm) << (A.shamt & 0x1F); break; // SLL
		case 7: if (A.s1data < A.I_imm) { aluresult = 1; }             //SLT
				else { aluresult = 0; }break;
		case 8:aluresult = unsigned(A.I_imm) >> (A.shamt & 0x1F); break; //SRL
		}

		if (A.alures == 0)
		{
			A.zeroflag = 1;         //zeroflag is 1 when output is 0
		}

	}
	else {
		switch (A.alucontrol)
		{
		case 1: A.alures = A.s1data + A.s2data; break;                 //ADD
		case 2: A.alures = A.s1data - A.s2data; break;                 //SUB
		case 3: A.alures = A.s1data & A.s2data; break;                 //AND
		case 4: A.alures = A.s1data || A.s2data; break;                //OR
		case 5: A.alures = A.s1data ^ A.s2data; break;                //XOR
		case 6: A.alures = signed(A.s2data) << (A.shamt & 0x1F); break; // SLL
		case 7: if (A.s1data < A.s2data) { A.alures = 1; }             //SLT
				else { A.alures = 0; }break;
		case 8:A.alures = unsigned(A.s2data) >> (A.shamt & 0x1F); break; //SRL
		}

		if (A.alures == 0)
		{
			A.zeroflag = 1;         //zeroflag is 1 when output is 0
		}
	}
}
void datamemoryreset()                   //initialize all the values in the memory to 0
{
	for (int i = 0; i < 32; i++)
	{
		memory[i] = 0;
	}
}
void datamemory(instWord &A)             //Read and Write in memory, memory size is 32
{
	uint32_t datamemresult = 0;
	if ((A.memorywrite) && (A.alures < 32))        //write
	{
		memory[A.alures] = A.rs2;

	}

	else
		if (!(A.memorywrite) && (A.alures<32))          //Read
		{
			datamemresult = memory[A.alures];
			A.datamemoryresult = datamemresult;
		}

}
void checkpc(uint32_t add, uint32_t target, instWord A)         //checks if pc exsits in the Branch Predictor Table
{
	bool flag = false;
	if ((add < A.branch.size()))
	{
		for (int i = 0; i < A.branch.size(); i++)
		{
			if (A.branch[i].pc == add)
				flag = true;
		}
	}
	if (!(flag))            //if pc doesn't exist add it
	{
		BranchPredictor addedpc;
		addedpc.pc = add;
		addedpc.targetaddress = target;
		addedpc.state = 0;          //State is strongly Not Taken
		A.branch.push_back(addedpc);           //  add the pc to the branch predictor
	}
}
void updatestate(uint32_t pc, uint32_t targetadd, bool currentstate, instWord A)   //updates the decision of the predictor based on the current state
{ //if current state is 1: branch taken, else not taken


	for (int i = 0; i < A.branch.size(); i++)
	{
		if (A.branch[i].pc == pc)
		{
			if (currentstate)
			{
				if (A.branch[i].state < 3)
				{
					A.branch[i].state = A.branch[i].state + 1;
				}
			}
			else if (!(currentstate))
			{
				if (A.branch[i].state > 0)
				{
					A.branch[i].state = A.branch[i].state - 1;
				}
			}
		}
	}
}

int get_state(uint32_t address, instWord A)      //returns the state of an address
{
	for (int i = 0; i < A.branch.size(); i++)
	{
		if (address == A.branch[i].pc)
		{
			return A.branch[i].state;
		}
	}
	return 0;      //not found so it will have a state of Strongly NT i.e 0
}
void decoder(instWord & instruction)
{
	instruction.I_imm = (instruction.instMachineCode & 0xFFFF);
	instruction.I_imm |= ((instruction.instMachineCode >> 15) & 1) ? 0xFFFF0000 : 0x0;
	instruction.rs1 = (instruction.instMachineCode >> 21) & 0x1F;
	instruction.rs2 = (instruction.instMachineCode >> 16) & 0x1F;
	instruction.rd = (instruction.instMachineCode >> 11) & 0x1F;
	instruction.funct = instruction.instMachineCode & 0x3F;
	instruction.opcode = (instruction.instMachineCode >> 26) & 0x3F;
	//instruction.pc = pc * 4;
	//    instruction.J_imm = (instruction.instMachineCode & 0xFFFF); //fjjkfl


}
uint32_t get_target_address(uint32_t pc1, instWord A)
{
	for (int i = 0; i < A.branch.size(); i++)
	{
		if (pc1 == A.branch[i].pc)
		{
			return A.branch[i].targetaddress;
		}
	}
	return pc1;  //not found so target address is the address itself
}
void  control_unit(instWord & A)
{
	switch (A.opcode)
	{
	case 0: {                                                                  //R-type
		switch (A.funct)
		{
		case 32:    A.alucontrol = 1; break;    //ADD
		case 38:    A.alucontrol = 5;  break;   //XOR
		case 42:    A.alucontrol = 7;  break;   //SLT
		case 8:     A.alucontrol = 1; break;     //JR

		}
		A.regwrite = A.regdst = true;
		A.ALUSrc = A.Branch = A.memorywrite = A.memtoreg = A.jump = A.Branchequal = false; break;
	}
	case 8:     // I-type--ADDI
	{
		//ADDI
		A.regwrite = A.ALUSrc = true;
		A.alucontrol = 1;// <-
		A.regdst = A.Branch = A.memorywrite = A.memtoreg = A.jump = A.Branchequal = false; break;
	}
	case 35:
	{                               //LW
		A.alucontrol = 1;
		A.regwrite = A.ALUSrc = A.memtoreg = true;
		A.regdst = A.Branch = A.memorywrite = A.jump = A.Branchequal = false; break;

	}
	case 43:
	{                                     //SW
		A.alucontrol = 1;
		A.ALUSrc = A.memorywrite = true;
		A.regwrite = A.regdst = A.Branch = A.jump = A.memtoreg = A.Branchequal = false; break;

	}
	case 2:
	{   A.alucontrol = 1;              //Jump
	A.jump = true;



	A.regdst = A.Branch = A.memorywrite = A.memtoreg = A.Branchequal = A.regwrite = A.ALUSrc = false; break;

	}
	}

}
void printRegs() {
	cout << "REGS\n";
	for (int i = 0; i<15; i++)
	{
		cout << "x" << i << "\t: " << regs[i] << endl;
	}
	cout << "MEM\n";
	for (int i = 0; i<32; i++)
	{
		cout << "x" << i << "\t: " << memory[i] << endl;
	}
}
