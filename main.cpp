#include <iostream>
#include <string>
#include <vector>
#include <queue>

using namespace std;

uint32_t    regs[15],
            memory[32],
            clk;
//bool regwrite, regdst, ALUSrc, Branch, memorywrite, memtoreg, jump, Branchequal;

struct BranchPredictor
{
    uint32_t pc;
    uint32_t targetaddress;
    int state;      //state 0: Stronglt NT --state 1: NT -- State 2: Taken -- State 3: Strongly Taken
};

struct instWord
{
    string instText;
    unsigned int instMachineCode;
    signed int rd, rs1, rs2, shamt, funct, opcode, alucontrol;
    signed int I_imm, S_imm, B_imm, U_imm, J_imm;
    unsigned pc;
    int zeroflag;
    uint32_t s1data, s2data;
    vector <BranchPredictor> branch;
    bool regwrite , ALUSrc , memtoreg ,regdst , Branch , memorywrite , jump , Branchequal;
    
};

void parse(instWord &W) {
    
    string temp = W.instText.substr(0, W.instText.find("\t"));
    
    
    if (temp != "main:\r") {
        if (temp == "add") {            //R Type
            W.opcode = 51;
            W.shamt = 0;
            W.funct = 0;
            W.instText.erase(0, 4);
            W.rd = stoi(W.instText.substr(1, W.instText.find(",") - 1));
            W.instText.erase(0, W.instText.find(",") + 2);
            W.rs1 = stoi(W.instText.substr(0, W.instText.find(",")));
            W.instText.erase(0, W.instText.find(",") + 2);
            W.rs2 = stoi(W.instText.substr(0, W.instText.find("\t")));
            W.instMachineCode = W.funct << 25;
            W.instMachineCode |= W.rs2 << 20;
            W.instMachineCode |= W.rs1 << 15;
            W.instMachineCode |= W.shamt << 12;
            W.instMachineCode |= W.rd << 7;
            W.instMachineCode |= W.opcode;
        }
        else if (temp == "slt") {
            W.opcode = 51;
            W.shamt = 2;
            W.funct = 0;
            W.instText.erase(0, 4);
            W.rd = stoi(W.instText.substr(1, W.instText.find(",") - 1));
            W.instText.erase(0, W.instText.find(",") + 2);
            W.rs1 = stoi(W.instText.substr(0, W.instText.find(",")));
            W.instText.erase(0, W.instText.find(",") + 2);
            W.rs2 = stoi(W.instText.substr(0, W.instText.find("\t")));
            
            W.instMachineCode = W.funct << 25;
            W.instMachineCode |= W.rs2 << 20;
            W.instMachineCode |= W.rs1 << 15;
            W.instMachineCode |= W.shamt << 12;
            W.instMachineCode |= W.rd << 7;
            W.instMachineCode |= W.opcode;
        }
        else if (temp == "xor") {
            W.opcode = 51;
            W.shamt = 4;
            W.funct = 0;
            W.instText.erase(0, 4);
            W.rd = stoi(W.instText.substr(1, W.instText.find(",") - 1));
            W.instText.erase(0, W.instText.find(",") + 2);
            W.rs1 = stoi(W.instText.substr(0, W.instText.find(",")));
            W.instText.erase(0, W.instText.find(",") + 2);
            W.rs2 = stoi(W.instText.substr(0, W.instText.find("\t")));
            
            W.instMachineCode = W.funct << 25;
            W.instMachineCode |= W.rs2 << 20;
            W.instMachineCode |= W.rs1 << 15;
            W.instMachineCode |= W.shamt << 12;
            W.instMachineCode |= W.rd << 7;
            W.instMachineCode |= W.opcode;
        }
        else if (temp == "addi") {     //I format
            W.opcode = 0x8;
            W.shamt = 0;
            W.instText.erase(0, W.instText.find("x"));
            W.rd = stoi(W.instText.substr(1, W.instText.find(",")));
            W.instText.erase(0, W.instText.find(",") + 1);
            W.rs1 = stoi(W.instText.substr(1, W.instText.find(",")));
            W.instText.erase(0, W.instText.find(",") + 1);
            W.I_imm = stoi(W.instText);
            cout << "Assembler rd = " << W.rd << endl;
            W.instMachineCode = W.I_imm << 20;
            W.instMachineCode |= W.rs1 << 15;
            W.instMachineCode |= W.shamt << 12;
            W.instMachineCode |= W.rd << 7;
            W.instMachineCode |= W.opcode;
        }
        else if (temp == "lw")
        {
            W.opcode = 3;
            W.shamt = 2;
            W.instText.erase(0, W.instText.find("x"));
            W.rd = stoi(W.instText.substr(1, W.instText.find(",")));
            W.instText.erase(0, W.instText.find(",") + 1);
            W.I_imm = stoi(W.instText.substr(0, W.instText.find("(")));
            W.instText.erase(0, W.instText.find("(") + 1);
            W.rs1 = stoi(W.instText.substr(1, W.instText.find(")") - 1));
            
            W.instMachineCode = W.I_imm << 20;
            W.instMachineCode |= W.rs1 << 15;
            W.instMachineCode |= W.shamt << 12;
            W.instMachineCode |= W.rd << 7;
            W.instMachineCode |= W.opcode;
        }
        else if (temp == "sw")
        {
            W.opcode = 35;
            W.shamt = 2;
            W.instText.erase(0, W.instText.find("x"));
            W.rs2 = stoi(W.instText.substr(1, W.instText.find(",")));
            W.instText.erase(0, W.instText.find(",") + 1);
            W.S_imm = stoi(W.instText.substr(0, W.instText.find("(")));
            W.rd = W.S_imm & 0b00000000000000000000000000011111;
            W.funct = W.S_imm & 0b00000000000000000000111111100000;
            W.instText.erase(0, W.instText.find("(") + 1);
            W.rs1 = stoi(W.instText.substr(1, W.instText.find(")") - 1));
            
            W.instMachineCode = W.funct << 25;
            W.instMachineCode |= W.rs2 << 20;
            W.instMachineCode |= W.rs1 << 15;
            W.instMachineCode |= W.shamt << 12;
            W.instMachineCode |= W.rd << 7;
            W.instMachineCode |= W.opcode;
        }
        
    }
}
/*struct contentsofbuffer
 {
 vector <uint32_t> currentdata;
 vector <uint32_t> nextdata;
 bool reset, enable;
 };
 struct buffers
 {
 vector <uint32_t> in;
 vector <uint32_t> out;
 contentsofbuffer buffer;
 };
 buffers b1;
 */


void Registersreset()                              //initialize all registers to 0
{
    for (int i = 0; i < 15; i++)
    {
        regs[i] = 0;
    }
}

void RegisterFile(instWord &A, uint32_t w_data)    //Read and Write from registers
{
    
    if ((A.regwrite) && (A.rd<16) && ((A.opcode == 0x8) || (A.opcode == 0x23)))
    {
        regs[A.rs2] = w_data;
        //Write
    }
    else
        if ((A.regwrite) && (A.rd<16) && (A.opcode == 0x20))
        {
            regs[A.rd] = w_data;
        }
    
    if (!(A.regwrite) && (A.rs1<16) && (A.rs2<16))
    {
        A.s1data = regs[A.rs1];                             //Read
        A.s2data = regs[A.rs2];
    }
}


/*void instructionmemoryinitialize(uint32_t ad, uint32_t ins)
 {
 instructionmemory addedinstruction;
 addedinstruction.address = ad;          //adding the address of the instruction
 addedinstruction.instr= ins;        //adding the instruction itself
 instr.push_back(addedinstruction);
 }
 uint32_t returninstruction(uint32_t ad)
 {
 
 for (int i = 0; i < instr.size(); i++)
 {
 if (instr[i].address == ad)
 {
 instruction = instr[i].instr;
 break;
 }
 }
 return 0;
 }
 */
int ALU(uint32_t operand1, uint32_t operand2, uint32_t shamt, int alucontrol, instWord &A)
{
    int aluresult = 0;
    switch (alucontrol)
    {
        case 1: aluresult = operand1 + operand2; break;                 //ADD
        case 2: aluresult = operand1 - operand2; break;                 //SUB
        case 3: aluresult = operand1 & operand2; break;                 //AND
        case 4: aluresult = operand1 || operand2; break;                //OR
        case 5: aluresult = operand1 ^ operand2; break;                //XOR
        case 6: aluresult = signed(operand2) << (shamt & 0x1F); break; // SLL
        case 7: if (operand1 < operand2) { aluresult = 1; }             //SLT
        else { aluresult = 0; }break;
        case 8:aluresult = unsigned(operand2) >> (shamt & 0x1F); break; //SRL
    }
    
    if (aluresult == 0)
    {
        A.zeroflag = 1;         //zeroflag is 1 when output is 0
    }
    return aluresult;
}
void datamemoryreset()                   //initialize all the values in the memory to 0
{
    for (int i = 0; i < 32; i++)
    {
        memory[i] = 0;
    }
}


uint32_t datamemory(instWord &A, uint32_t address)             //Read and Write in memory, memory size is 32
{
    uint32_t datamemresult = 0;
    if ((A.memorywrite) && (address < 32))        //write
    {
        memory[address] = A.rs2;
        return 0;
    }
    
    else
        if (!(A.memorywrite) && (address<32))          //Read
        {
            datamemresult = memory[address];
            return datamemresult;
        }
    return 0;
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

void decoder(instWord & instruction, unsigned pc)
{
    instruction.I_imm = (instruction.instMachineCode & 0xFFFF);
    instruction.I_imm |= ((instruction.instMachineCode >> 15) & 1) ? 0xFFFF0000 : 0x0;
    instruction.rs1 = (instruction.instMachineCode >> 21) & 0x1F;
    instruction.rs2 = (instruction.instMachineCode >> 16) & 0x1F;
    instruction.rd = (instruction.instMachineCode >> 11) & 0x1F;
    instruction.funct = instruction.instMachineCode & 0x3F;
    instruction.opcode = (instruction.instMachineCode >> 26) & 0x3F;
    instruction.pc = pc * 4;
    //    instruction.J_imm = (instruction.instMachineCode & 0xFFFF); //fjjkfl
    
    
}
void  control_unit(instWord A)
{
    int alucontrol = 0;
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
            alucontrol = 1;        //ADDI
            A.regwrite = A.ALUSrc = true;
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
bool stall(uint32_t rs1_ex, uint32_t rs2_ex, uint32_t rs2_df, bool memtoreg_df, uint32_t rs2_ds, bool memtoreg_ds)   //gets the decision whether to stall or continue
{
    if (memtoreg_df) {
        if (rs1_ex == rs2_df || rs2_ex == rs2_df)
            if (rs2_df != 0)
                return true;
    }
    if (memtoreg_ds) {
        if (rs1_ex == rs2_ds || rs2_ex == rs2_ds)
            if (rs2_ds != 0)
                return true;
    }
    
    return false;
}
//foward EX: 0 DF:1 DS:2 TCmem:3 TCalu:4
int fowardingunit(uint32_t rs_exec, uint32_t rd_df, bool reg_we_DF, uint32_t rd_DS, bool reg_we_DS, uint32_t rd_TC, bool reg_we_TC, uint32_t rt_TC, bool mem_to_reg_TC)
{
    
    if (rs_exec == 0)
        return 0;   //EX
    
    if (reg_we_DF && rs_exec == rd_df)
        return 1;   //DF
    
    if (reg_we_DS && rs_exec == rd_DS)
        return 2;  //DS
    
    if (reg_we_TC && mem_to_reg_TC && rs_exec == rt_TC)
        return 3;  //TCmem
    
    if (reg_we_TC && rs_exec == rd_TC)
        return 4;  //TCalu
    
    return 0;
    
}
void printRegs(){
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

void nextclk(instWord W[], int i) {
    /*
     decoder(W[clk] );
     RegisterFile( ); W[clk-1]       //Zeyad
     ALU(); W[clk-2]
     DataMemory() W[clk-3] */
    clk++;
}

void signExtend(instWord & W){
    if (W.I_imm >> 15)
       W.I_imm += 0xFFFF0000;
}

void init(){
    
    clk = 0;
    Registersreset();
    
    
}

int main(int argc, char *argv[])
{
    init();
    int  alures;
    

    instWord W;
    //ROM
    decoder(W, 0);
    RegisterFile(W, 0);
    signExtend(W);
    if (W.ALUSrc)   alures = ALU(W.s1data, W.I_imm, W.shamt, W.alucontrol, W);
    else            alures = ALU(W.s1data, W.s2data, W.shamt, W.alucontrol, W);
    
    
    if (W.memtoreg) RegisterFile(W, datamemory(W, alures));
    else            RegisterFile(W, alures);
    
    printRegs();
    
    return 0;
    
}
