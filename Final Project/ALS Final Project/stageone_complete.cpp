#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

typedef struct gate{
    string name;
    int type;
    int level;
    int andorinv;
    vector<gate> inputs;
}gate;

typedef struct model{
    string name;
    vector<gate> node;
}model;

gate temp_gate;
model temp_model;

vector<gate>::iterator it_gate;

ifstream ifs("test.blif");
ofstream ofs("my_test.blif");

string line_temp, segment_temp;

vector<gate> temp_inputs;
vector<string> inputnode;
vector<string> outputnode;


int current_read_op = -1;

int numberofinputs = 0;
int numberofoutputs = 0;
int gate_number = 0;

void readfiles();
void stageone();
void output();

int main()
{
    readfiles();
    stageone();
    output();
    ifs.close();
    ofs.close();
    return 0;
}

void readfiles()
{
    while(getline(ifs, line_temp))
    {
        stringstream ss(line_temp);
        getline(ss, segment_temp, ' ');
        if(segment_temp == ".model")
        {
            current_read_op = 1;
            getline(ss, segment_temp, ' ');
            temp_model.name = segment_temp;
        }
        else if(segment_temp == ".inputs")
        {
            current_read_op = 2;
            while(getline(ss, segment_temp, ' '))
            {
                if(segment_temp != "\\")
                {
                    temp_gate.name = segment_temp;
                    temp_gate.type = 1;
                    temp_gate.level = 0;
                    temp_gate.andorinv = -1;
                    temp_model.node.push_back(temp_gate);
                    inputnode.push_back(segment_temp);
                    numberofinputs++;
                }
            }
        }
        else if(segment_temp == ".outputs")
        {
            current_read_op = 3;
            while(getline(ss, segment_temp, ' '))
            {
                if(segment_temp != "\\")
                {
                    temp_gate.name = segment_temp;
                    temp_gate.type = 3;
                    temp_model.node.push_back(temp_gate);
                    outputnode.push_back(segment_temp);
                    numberofoutputs++;
                }
            }
        }
        else if(segment_temp == ".names")
        {
            int max_level = -1;
            current_read_op = 4;
            while(getline(ss, segment_temp, ' '))
            {
                gate_number = 0;
                for(gate_number=0;gate_number<temp_model.node.size();gate_number++)
                {
                    if(temp_model.node[gate_number].name == segment_temp)
                    {
                        break;
                    }
                }
                if(gate_number != temp_model.node.size())
                {
                    if(temp_model.node[gate_number].type == 3)
                    {
                        temp_model.node[gate_number].inputs = temp_inputs;
                        temp_model.node[gate_number].level = max_level+1;
                        temp_inputs.clear();
                    }
                    else
                    {
                        temp_inputs.push_back(temp_model.node[gate_number]);
                        if(temp_model.node[gate_number].level > max_level)
                        {
                            max_level = temp_model.node[gate_number].level;
                        }
                    }
                }
                else
                {
                    temp_gate.name = segment_temp;
                    temp_gate.type = 2;
                    temp_gate.level = max_level+1;
                    temp_gate.inputs = temp_inputs;
                    temp_inputs.clear();
                    temp_model.node.push_back(temp_gate);
                    temp_gate.inputs.clear();
                    gate_number = temp_model.node.size()-1;
                }
            }
        }
        else if(segment_temp == ".end")
        {
            current_read_op = 5;
        }
        else
        {
            if(current_read_op == 2)
            {
                temp_gate.name = segment_temp;
                temp_gate.type = 1;
                temp_gate.level = 0;
                temp_gate.andorinv = -1;
                temp_model.node.push_back(temp_gate);
                inputnode.push_back(segment_temp);
                numberofinputs++;
                while(getline(ss, segment_temp, ' '))
                {
                    if(segment_temp != "\\")
                    {
                        temp_gate.name = segment_temp;
                        temp_gate.type = 1;
                        temp_gate.level = 0;
                        temp_gate.andorinv = -1;
                        temp_model.node.push_back(temp_gate);
                        inputnode.push_back(segment_temp);
                        numberofinputs++;
                    }
                }
            }
            else if(current_read_op == 3)
            {
                temp_gate.name = segment_temp;
                temp_gate.type = 3;
                temp_model.node.push_back(temp_gate);
                outputnode.push_back(segment_temp);
                numberofoutputs++;
                while(getline(ss, segment_temp, ' '))
                {
                    if(segment_temp != "\\")
                    {
                        temp_gate.name = segment_temp;
                        temp_gate.type = 3;
                        temp_model.node.push_back(temp_gate);
                        outputnode.push_back(segment_temp);
                        numberofoutputs++;
                    }
                }
            }
            else if(current_read_op == 4)
            {
                int skipping_line = temp_model.node[gate_number].inputs.size();
                if(segment_temp[0] == '1' && segment_temp[1] == '1')
                {
                    temp_model.node[gate_number].andorinv = 1;
                }
                else if(segment_temp[0] == '1' && segment_temp[1] == '-')
                {
                    temp_model.node[gate_number].andorinv = 2;
                }
                else if(segment_temp[0] == '0')
                {
                    temp_model.node[gate_number].andorinv = 3;
                }
            }
        }
    }
}

bool compareInterval(gate i1, gate i2)
{
    return (i1.level < i2.level);
}

void stageone()
{
    // Haven't Sort Using Toplogical Order, causing overflow values at level.
    // Record In Out String for Output Use
    //temp_gate.inputs.clear();
    int i,j,k;
    int node_size = temp_model.node.size();
    sort(temp_model.node.begin(),temp_model.node.end(), compareInterval);
    for(i=numberofinputs;i<node_size;i++)
    {
        for(j=0;j<temp_model.node[i].inputs.size();j++)
        {
            for(k=0;k<node_size;k++)
            {
                if(temp_model.node[k].name == temp_model.node[i].inputs[j].name)
                {
                    temp_model.node[i].inputs[j].level = temp_model.node[k].level;
                }
            }
        }
        int counter = 0;
        while(temp_model.node[i].inputs.size() > 2)
        {
            counter++;
            sort(temp_model.node[i].inputs.begin(),temp_model.node[i].inputs.end(), compareInterval);
            temp_gate.name = temp_model.node[i].name + to_string(counter);
            temp_gate.andorinv = temp_model.node[i].andorinv;
            temp_gate.level = max(temp_model.node[i].inputs[0].level, temp_model.node[i].inputs[1].level)+1;
            temp_gate.inputs.push_back(temp_model.node[i].inputs[0]);
            temp_gate.inputs.push_back(temp_model.node[i].inputs[1]);
            temp_model.node.push_back(temp_gate);
            temp_model.node[i].inputs.push_back(temp_gate);
            temp_gate.inputs.clear();
            temp_model.node[i].inputs.erase(temp_model.node[i].inputs.begin());
            temp_model.node[i].inputs.erase(temp_model.node[i].inputs.begin());
        }
        if(temp_model.node[i].andorinv == 3)
        {
            temp_model.node[i].level = temp_model.node[i].inputs[0].level+1;
        }
        else
        {
            temp_model.node[i].level = max(temp_model.node[i].inputs[0].level, temp_model.node[i].inputs[1].level)+1;
        }
    }
}

void output()
{
    int i,j;
    ofs << ".model " << temp_model.name << endl;
    ofs << ".inputs";
    for(i=0;i<numberofinputs;i++)
    {
        ofs << " " << inputnode[i];
        //ofs << "(" << temp_model.node[i].level << ")";
    }
    ofs << endl;
    ofs << ".outputs";
    for(i=0;i<numberofoutputs;i++)
    {
        ofs << " " <<outputnode[i];
        //ofs << "(" << temp_model.node[numberofinputs+i].level << ")";
    }
    ofs << endl;
    for(i=numberofinputs;i<temp_model.node.size();i++)
    {
        ofs << ".names";
        for(j=0;j<temp_model.node[i].inputs.size();j++)
        {
            ofs << " " << temp_model.node[i].inputs[j].name;
            //ofs << "(" << temp_model.node[i].inputs[j].level << ")";
        }
        ofs << " " << temp_model.node[i].name;
        //ofs << "(" << temp_model.node[i].level << ")";
        ofs << endl;
        if(temp_model.node[i].andorinv == 1)
        {
            ofs << "11 1" << endl;
        }
        else if(temp_model.node[i].andorinv == 2)
        {
            ofs << "1- 1" << endl;
            ofs << "-1 1" << endl;
        }
        else if(temp_model.node[i].andorinv == 3)
        {
            ofs << "0 1" << endl;
        }
    }
    ofs << ".end" << endl;
}
