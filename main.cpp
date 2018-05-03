#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <fstream>

using namespace std;

uint32_t    regs[15],
            s1data,            //Not needed anymore
            s2data,        //Not needed anymore
            aluresult,             //Not needed anymore
            memory[32],
            opcode,       //Not needed anymore
            RS,       //Not needed anymore
            RT,       //Not needed anymore
            RD,        //Not needed anymore
            func,       //Not needed anymore
            imm,        //Not needed anymore
            clk;
int         zeroflag=0, //TO BE ADDED TO INSTWORD STRUCT
            datamemresult,  //SAME
            alucontrol;     //SAME
bool regwrite,regdst,ALUSrc,Branch,memorywrite,memtoreg,jump, Branchequal;


struct instWord
{
    string instText;
    unsigned int instMachineCode;
    signed int rd, rs1, rs2, funct3, funct7, opcode;
    signed int I_imm, S_imm, B_imm, U_imm, J_imm;
    unsigned pc;
};

void parse(instWord &W) {
    
    string temp = W.instText.substr(0, W.instText.find("\t"));
    
    
    if (temp != "main:\r") {
        if (temp == "add") {            //R Type
            W.opcode = 51;
            W.funct3 = 0;
            W.funct7 = 0;
            W.instText.erase(0, 4);
            W.rd = stoi(W.instText.substr(1, W.instText.find(",") - 1));
            W.instText.erase(0, W.instText.find(",") + 2);
            W.rs1 = stoi(W.instText.substr(0, W.instText.find(",")));
            W.instText.erase(0, W.instText.find(",") + 2);
            W.rs2 = stoi(W.instText.substr(0, W.instText.find("\t")));
            W.instMachineCode = W.funct7 << 25;
            W.instMachineCode |= W.rs2 << 20;
            W.instMachineCode |= W.rs1 << 15;
            W.instMachineCode |= W.funct3 << 12;
            W.instMachineCode |= W.rd << 7;
            W.instMachineCode |= W.opcode;
        }
        else if (temp == "slt") {
            W.opcode = 51;
            W.funct3 = 2;
            W.funct7 = 0;
            W.instText.erase(0, 4);
            W.rd = stoi(W.instText.substr(1, W.instText.find(",") - 1));
            W.instText.erase(0, W.instText.find(",") + 2);
            W.rs1 = stoi(W.instText.substr(0, W.instText.find(",")));
            W.instText.erase(0, W.instText.find(",") + 2);
            W.rs2 = stoi(W.instText.substr(0, W.instText.find("\t")));
            
            W.instMachineCode = W.funct7 << 25;
            W.instMachineCode |= W.rs2 << 20;
            W.instMachineCode |= W.rs1 << 15;
            W.instMachineCode |= W.funct3 << 12;
            W.instMachineCode |= W.rd << 7;
            W.instMachineCode |= W.opcode;
        }
        else if (temp == "xor") {
            W.opcode = 51;
            W.funct3 = 4;
            W.funct7 = 0;
            W.instText.erase(0, 4);
            W.rd = stoi(W.instText.substr(1, W.instText.find(",") - 1));
            W.instText.erase(0, W.instText.find(",") + 2);
            W.rs1 = stoi(W.instText.substr(0, W.instText.find(",")));
            W.instText.erase(0, W.instText.find(",") + 2);
            W.rs2 = stoi(W.instText.substr(0, W.instText.find("\t")));
            
            W.instMachineCode = W.funct7 << 25;
            W.instMachineCode |= W.rs2 << 20;
            W.instMachineCode |= W.rs1 << 15;
            W.instMachineCode |= W.funct3 << 12;
            W.instMachineCode |= W.rd << 7;
            W.instMachineCode |= W.opcode;
        }
        else if (temp == "addi") {     //I format
            W.opcode = 0x8;
            W.funct3 = 0;
            W.instText.erase(0, W.instText.find("x"));
            W.rd = stoi(W.instText.substr(1, W.instText.find(",")));
            W.instText.erase(0, W.instText.find(",")+1);
            W.rs1 = stoi(W.instText.substr(1, W.instText.find(",")));
            W.instText.erase(0, W.instText.find(",")+1);
            W.I_imm = stoi(W.instText);
            cout << "Assembler rd = " << W.rd << endl;
            W.instMachineCode = W.I_imm << 20;
            W.instMachineCode |= W.rs1 << 15;
            W.instMachineCode |= W.funct3 << 12;
            W.instMachineCode |= W.rd << 7;
            W.instMachineCode |= W.opcode;
        }
        else if (temp == "lw")
        {
            W.opcode = 3;
            W.funct3 = 2;
            W.instText.erase(0, W.instText.find("x"));
            W.rd = stoi(W.instText.substr(1, W.instText.find(",")));
            W.instText.erase(0, W.instText.find(",") + 1);
            W.I_imm = stoi(W.instText.substr(0, W.instText.find("(")));
            W.instText.erase(0, W.instText.find("(") + 1);
            W.rs1 = stoi(W.instText.substr(1, W.instText.find(")") - 1));
            
            W.instMachineCode = W.I_imm << 20;
            W.instMachineCode |= W.rs1 << 15;
            W.instMachineCode |= W.funct3 << 12;
            W.instMachineCode |= W.rd << 7;
            W.instMachineCode |= W.opcode;
        }
        else if (temp == "sw")
        {
            W.opcode = 35;
            W.funct3 = 2;
            W.instText.erase(0, W.instText.find("x"));
            W.rs2 = stoi(W.instText.substr(1, W.instText.find(",")));
            W.instText.erase(0, W.instText.find(",") + 1);
            W.S_imm = stoi(W.instText.substr(0, W.instText.find("(")));
            W.rd = W.S_imm & 0b00000000000000000000000000011111;
            W.funct7 = W.S_imm & 0b00000000000000000000111111100000;
            W.instText.erase(0, W.instText.find("(") + 1);
            W.rs1 = stoi(W.instText.substr(1, W.instText.find(")") - 1));
            
            W.instMachineCode = W.funct7 << 25;
            W.instMachineCode |= W.rs2 << 20;
            W.instMachineCode |= W.rs1 << 15;
            W.instMachineCode |= W.funct3 << 12;
            W.instMachineCode |= W.rd << 7;
            W.instMachineCode |= W.opcode;
        }
        
    }
}
struct contentsofbuffer
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

struct BranchPredictor
{
    uint32_t pc;
    uint32_t targetaddress;
    int state;      //state 0: Stronglt NT --state 1: NT -- State 2: Taken -- State 3: Strongly Taken
};
vector <BranchPredictor> branch;


void Registersreset()                              //initialize all registers to 0
{
    for (int i = 0; i < 15; i++)
    {
        regs[i] = 0;
    }
}

void RegisterFile (uint32_t rs, uint32_t rt, uint32_t rd, uint32_t w_data)    //Read and Write from registers
{
    
    if ((regwrite) && (rd<16))
    {
        regs[rd] = w_data;                                    //Write
    }
    else
        if (!(regwrite) && (rs<16) && (rt<16))
        {
            s1data = regs[rs];                             //Read
            s2data = regs[rt];
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

void ALU(uint32_t operand1, uint32_t operand2, uint32_t shamt)
{
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
    
    if (aluresult==0)
    {
        zeroflag = 1;         //zeroflag is 1 when output is 0
    }
}

void datamemoryreset()                   //initialize all the values in the memory to 0
{
    for (int i = 0; i < 32; i++)
    {
        memory[i] = 0;
    }
}
void datamemory(uint32_t address, uint32_t data)             //Read and Write in memory, memory size is 32
{
    if ((memorywrite) && (address < 32))        //write
    {
        memory[address] = data;
        
    }
    
    else
        if (!(memorywrite) && (address<32))          //Read
        {
            datamemresult = memory[address];
        }
}

void checkpc(uint32_t add, uint32_t target)         //checks if pc exsits in the Branch Predictor Table
{
    bool flag = false;
    if ((add < branch.size()))
    {
        for (int i = 0; i < branch.size(); i++)
        {
            if (branch[i].pc == add)
                flag = true;
            
        }
    }
    
    if (!(flag))            //if pc doesn't exist add it
    {
        BranchPredictor addedpc;
        addedpc.pc = add;
        addedpc.targetaddress = target;
        addedpc.state = 0;          //State is strongly Not Taken
        branch.push_back(addedpc);           //  add the pc to the branch predictor
    }
    
    
}
void updatestate(uint32_t pc, uint32_t targetadd,bool currentstate)   //updates the decision of the predictor based on the current state
{ //if current state is 1: branch taken, else not taken
    
    
    for (int i = 0; i < branch.size(); i++)
    {
        if (branch[i].pc == pc)
        {
            if (currentstate)
            {
                if (branch[i].state < 3)
                {
                    branch[i].state = branch[i].state + 1;
                }
            }
            else if (!(currentstate))
            {
                if (branch[i].state > 0)
                {
                    branch[i].state = branch[i].state - 1;
                }
            }
        }
    }
}
int get_state(uint32_t address)      //returns the state of an address
{
    for (int i = 0; i < branch.size(); i++)
    {
        if (address == branch[i].pc)
        {
            return branch[i].state;
        }
    }
    return 0;      //not found so it will have a state of Strongly NT i.e 0
}
uint32_t get_target_address(uint32_t pc1)
{
    for (int i = 0; i < branch.size(); i++)
    {
        if (pc1 == branch[i].pc)
        {
            return branch[i].targetaddress;
        }
    }
    return pc1;  //not found so target address is the address itself
}
*/
void decoder(instWord & instruction, unsigned pc)
{
    instruction.I_imm = (instruction.instMachineCode & 0xFFFF);
    instruction.I_imm |= ((instruction.instMachineCode >> 15) & 1) ? 0xFFFF0000 : 0x0;
    instruction.rs1 = (instruction.instMachineCode >> 21) & 0x1F;
    instruction.rs2 = (instruction.instMachineCode >> 16) & 0x1F;
    instruction.rd =  (instruction.instMachineCode >> 11) & 0x1F;
    instruction.funct7 = instruction.instMachineCode & 0x3F;
    instruction.opcode = (instruction.instMachineCode >> 26) & 0x3F;
    instruction.pc = pc * 4;
    
    
}
void control_unit()
{
    switch (opcode)
    {
        case 0: {                                                                  //R-type
            switch (func)
            {
                case 32: alucontrol = 1; break;    //ADD
                case 38: alucontrol = 5;  break;   //XOR
                case 42: alucontrol = 7;  break;   //SLT
                case 8: alucontrol = 1; break;     //JR
                    
            }
            regwrite = regdst = true;
            ALUSrc = Branch = memorywrite = memtoreg = jump = Branchequal = false; break;
        }
        case 8:     // I-type--ADDI
        {
            alucontrol = 1;        //ADD
            regwrite = ALUSrc = true;
            regdst = Branch = memorywrite = memtoreg = jump = Branchequal = false; break;
        }
        case 35:
        {                               //LW
            alucontrol = 1;
            regwrite = ALUSrc = memtoreg = true;
            regdst = Branch = memorywrite = jump = Branchequal = false; break;
        }
        case 43:
        {                                     //SW
            alucontrol = 1;
            ALUSrc = memorywrite = true;
            regwrite = regdst = Branch = jump = memtoreg = Branchequal = false; break;
            
        }
        case 2:
        {   alucontrol = 1;              //Jump
            jump = true;
            regdst = Branch = memorywrite = memtoreg = Branchequal = regwrite = ALUSrc = false; break;
            
        }
    }
}
bool stall(uint32_t rs1_ex, uint32_t rs2_ex, uint32_t rs2_df, bool memtoreg_df, uint32_t rs2_ds, bool memtoreg_ds)   //gets the decision whether to stall or continue
{
    if (memtoreg_df) {
        if (rs1_ex == rs2_df || rs2_ex == rs2_df)
            if (rs2_df!= 0)
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
    
    if (reg_we_DF && rs_exec== rd_df)
        return 1;   //DF
    
    if (reg_we_DS && rs_exec == rd_DS)
        return 2;  //DS
    
    if (reg_we_TC && mem_to_reg_TC && rs_exec == rt_TC)
        return 3;  //TCmem
    
    if (reg_we_TC && rs_exec == rd_TC)
        return 4;  //TCalu
    
    return 0;
    
}

/*void buffersetnextdata(vector <uint32_t> d)
 {
 b1.buffer.nextdata = d;
 
 }
 void buffersetdata(vector <uint32_t> &q)
 {
 q = b1.buffer.currentdata;   //q is the output
 }
 void buffereset(bool r)
 {
 b1.buffer.reset = r;
 }
 void bufferenable(bool en)
 {
 b1.buffer.enable = en;
 }
 
 void bufferupdate()
 {
 if (b1.buffer.reset)
 {
 b1.currentdata = 0;
 }
 
 else if (b1.enable)
 {
 b1.currentdata = b1.nextdata;
 }
 }
 */

void nextclk(instWord W[] ,int i){
    /*
    decoder(W[clk] );
    RegisterFile( ); W[clk-1]       //Zeyad
    ALU(); W[clk-2]
    DataMemory() W[clk-3] */
    clk ++;
}



int main(int argc, char *argv[])
{
    clk = 0;
    Registersreset();
    ifstream inFile;
    instWord W;
    queue <unsigned> ROM;
    inFile.open("div.s"); //FILE PATH GOES HERE:: HANA Please check this.
    if (inFile.is_open())
    {
        while (!inFile.eof()) {
            getline(inFile, W.instText);
            cout << W.instText << endl;
            parse(W);
            if (W.instMachineCode != 0)
                ROM.push(W.instMachineCode);            //Filling ROM
        }
        inFile.close();
    }
    unsigned ROMSize =  unsigned(ROM.size());
    instWord IM [ROMSize]; int i = 0;
    while (!ROM.empty()){                               //Splitting ROM
        IM[i].instMachineCode = ROM.front();
        ROM.pop();
    }

    //table.item(0,0)->setText("first");
    //table.item(0,2)->setText("right");
    //  table.item(2,2)->setText("last");
    
    // table.show();
    return 0;
    //a.exec();
}
