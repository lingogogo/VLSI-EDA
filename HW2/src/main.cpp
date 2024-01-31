#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include <queue>
#include <random>
#include<iomanip>
#include <climits>
using namespace std;

struct LibCell {
    string name;
    int x, y;
    int area;
};

struct Tech {
    string name;
    int numLibCells{};
    vector<LibCell> libCells;
};



struct DieSize {
    long double width, height;
};

struct DieAssignment {
    string dieName;
    string techName;
    long double utilization;
};


struct Cell {
    string name;
    string libCellName;
    int vertex_index;
    int partition = -1;
    long long both_area[2] = { 0,0 };
    vector<int> cell_net;
    bool lock = false;
};

struct Net {
    string name;
    int numberconnectedCell;
    vector<string> connectedCells;
    vector<int> edge_index;
    vector<int> vertex_index;
};

// global parameter
long double tot_area[2] = { 0,0 };
long tot_cut_size = 0;
vector<int> gain_stack_;
vector<Cell> move_stack_;

void create_hgr_file(vector<Net>& net, vector<Cell>& cells, string outputname)
{
    std::ofstream output_hgr(outputname);
    output_hgr << net.size() << " " << cells.size() << " " << endl;
    for (int i = 0; i < net.size(); i++) {
        for (int j = 0; j < net[i].vertex_index.size(); j++) {
            output_hgr << net[i].vertex_index[j] + 1 << " ";//C1 = 0, C2 = 1;
        }
        output_hgr << endl;
    }
    output_hgr.close();
}

//struct Cell_libcell {map<int, string> cell_table;}; // <index, partition>
void cal_area(vector<Cell>& cells, vector<Tech> techs)
{
    for (int i = 0; i < cells.size(); i++)
    {
        for (int k = 0; k < techs.size(); k++)
        {
            for (int j = 0; j < techs[k].libCells.size(); j++)
            {
                if (cells[i].libCellName == techs[k].libCells[j].name)
                {
                    cells[i].both_area[k] = techs[k].libCells[j].area;
                    if (techs.size() == 1)
                    {
                        cells[i].both_area[1] = cells[i].both_area[0];
                    }
                }
            }
        }
    }
}
//calculate whole area for two dies.
void whole_area(vector<Cell>& cells)
{
    for (auto it = cells.begin(); it != cells.end(); it++)
    {
        tot_area[it->partition] += cells[it->vertex_index].both_area[it->partition];
    }
}
// generate the *1.out file for the verify.
void output_file_verify(vector<Cell>& cells, vector<DieAssignment> dieAssignments, const char* output_file_verify, long min_cut_size) {
    int count[2] = { 0,0 };

    for (int i = 0; i < cells.size(); i++)
    {
        if (cells[i].partition == 0) count[0]++;
        else count[1]++;
    }
    std::ofstream output_out(output_file_verify);
    output_out << "CutSize" << " " << min_cut_size << endl;
    for (int i = 0; i < dieAssignments.size(); i++)
    {
        output_out << dieAssignments[i].dieName << " " << count[i] << endl;
        for (int j = 0; j < cells.size(); j++)
        {
            if (cells[j].partition == i)
                output_out << cells[j].name << endl;
        }
    }
}
// calculate the cutsize for the partition problem
void tot_cutsize(vector<Cell>& cells, vector<Net>& nets) {

    tot_cut_size = 0;
    for (int i = 0; i < nets.size(); i++)
    {
        for (int j = 1; j < nets[i].vertex_index.size(); j++)
        {
            if (cells[nets[i].vertex_index[0]].partition != cells[nets[i].vertex_index[j]].partition)
            {
                tot_cut_size++;
                break;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    const char* inputFilename = argv[1];

    const char* outputFile_verify = argv[2];
    std::cout << std::fixed;
    std::cout << std::setprecision(2);
    cout << "start" << endl;
    ifstream inputFile(inputFilename);
    string line;
    vector<Tech> techs;
    vector<LibCell> libcells;
    DieSize dieSize;
    vector<DieAssignment> dieAssignments;
    int numCells;
    vector<Cell> cells;
    int numNets;
    vector<Net> nets;
    int netnumber = 0;
    map<int, vector<int>> cell_net_index; // cell with the net index;
    int count_net = 0;

    while (getline(inputFile, line)) {
        istringstream iss(line);
        string keyword;
        string tech_name;
        iss >> keyword;
        //cout << "keyword: " << keyword << endl;
        if (keyword == "Tech")
        {
            Tech tech;
            int count_tech = 0;
            string techname;
            iss >> techname >> count_tech;
            tech.name = techname;
            tech.numLibCells = count_tech;
            //cout << techname << endl;
            for (int i = 0; i < count_tech; i++)
            {
                getline(inputFile, line);
                istringstream iss_4(line);
                LibCell libCell;
                string trash;
                iss_4 >> trash >> libCell.name >> libCell.x >> libCell.y;
                libCell.area = libCell.x * libCell.y;
                //cout << libCell.name << " " << libCell.area << endl;
                libcells.push_back(libCell);
                tech.libCells.push_back(libCell);
            }
            techs.push_back(tech);
        }
        else if (keyword == "DieSize") {
            iss >> dieSize.width >> dieSize.height;
        }
        else if (keyword == "DieA" || keyword == "DieB") {
            DieAssignment assignment;
            assignment.dieName = keyword;
            iss >> assignment.techName >> assignment.utilization;
            //cout << "Die utilizaiton: " << assignment.utilization << endl;
            dieAssignments.push_back(assignment);
        }
        else if (keyword == "Cell") {
            Cell cell;
            iss >> cell.name >> cell.libCellName;
            //cout << cell.name << " " << cell.libCellName << endl;
            int numericValue = 0;
            for (char c : cell.name) {
                if (std::isdigit(c)) {
                    // If the character is a digit, add it to the numericValue
                    numericValue = numericValue * 10 + (c - '0');
                }
            }
            cell.vertex_index = numericValue - 1;
            vector<int> temp;
            //cell_net_index[numericValue-1] = temp;
            cells.push_back(cell);
        }
        else if (keyword == "NumNets") {
            iss >> numNets;
        }
        else if (keyword == "Net") {

            Net net;
            net.edge_index.push_back(count_net);
            iss >> net.name >> net.numberconnectedCell;
            //cout << net.name << " " << net.numberconnectedCell  << endl;
            //cout << net.name << endl;
            for (int i = 0; i < net.numberconnectedCell; i++) {
                getline(inputFile, line);//必須透過getline才可以to下一行。
                istringstream iss_2(line);
                string connectedCell;
                string trash;
                iss_2 >> trash >> connectedCell;

                net.connectedCells.push_back(connectedCell);
                int cell_index = 0;
                // Iterate through the characters of the string
                for (char c : connectedCell) {
                    if (std::isdigit(c)) {
                        // If the character is a digit, add it to the numericValue
                        cell_index = cell_index * 10 + (c - '0');
                    }
                }
                //cout << "cell_index: " << cell_index << endl;
                //cell_net_index[cell_index-1].push_back(count_net);
                net.vertex_index.push_back(cell_index - 1);

            }
            count_net++;
            nets.push_back(net);
        }
    }

    inputFile.close();
    string outputFileName = "test.hgr";
    create_hgr_file(nets, cells, outputFileName);
    int iter = 30;
    long min_cut_size = LONG_MAX;
    vector<Cell> temp_cells;
    // calculate the cells in two different dies' area.
    cal_area(cells, techs);
    for (int k = 0; k < iter; k++)
    {
        //string temp = "../hMETIS/hmetis-1.5-linux/hmetis test" + to_string(iter) + " 2 20 1 1 1 0 0 1";
        const char* hmetisCommand = "../src/hMETIS/hmetis-1.5-linux/hmetis test.hgr 2 20 1 1 1 0 0 1";     
        //<GraphFile> <Nparts> <UBfactor> <Nruns> <Ctype> <RType> <VCycle> <Reconst> <dbglvl>
        int output = system(hmetisCommand);
        //std::cout << "hemetis end" << endl;

        ifstream answerFile("test.hgr.part.2");
        string line2;
        int cell_ind = 0;
        while (getline(answerFile, line2)) {
            istringstream iss2(line2);
            int partition_from_hgr;
            iss2 >> partition_from_hgr;
            cells[cell_ind].partition = partition_from_hgr;
            cell_ind++;
        }
        //know the everynet partition
        answerFile.close();
        tot_cutsize(cells, nets);
        std::cout << "original cutsize: " << tot_cut_size << endl;
        whole_area(cells);

        long double areaconstraint[2] = { 0,0 };
        // area_constraint
        for (int i = 0; i < dieAssignments.size(); i++)
        {
            std::cout << dieSize.height << " " << dieSize.width << " " << dieAssignments[i].utilization << endl;
            areaconstraint[i] = dieSize.height * dieSize.width * dieAssignments[i].utilization / 100;
        }

        std::cout << "areaA_constraint: " << areaconstraint[0] << "\n" << "areaB_constraint: " << areaconstraint[1] << endl;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> distribution(0, cells.size() - 1);
        std::uniform_int_distribution<int> distribution1(1, 100);

        // fulfill the area objective
        // deal with the problem. Start from DieA.
        while (tot_area[0] > areaconstraint[0] || tot_area[1] > areaconstraint[1])
        {
            int rand = distribution(gen);
            //std::cout << rand<<endl;
            int force_change = distribution1(gen);
            //if (tot_area[0] == old_tot_area[0] && tot_area[1] == old_tot_area[1] && count_lock >= 20)
            if (force_change < 8)
            {
                //std::cout << "random part" << endl;
                if (cells[rand].partition == 0)
                {
                    tot_area[0] -= cells[rand].both_area[0];
                    tot_area[1] += cells[rand].both_area[1];
                    cells[rand].partition = 1;
                }
                else {
                    tot_area[1] -= cells[rand].both_area[1];
                    tot_area[0] += cells[rand].both_area[0];
                    cells[rand].partition = 0;
                }
                continue;
            }
            if (tot_area[0] > areaconstraint[0] && tot_area[1] > areaconstraint[1] && cells[rand].partition == 1)
            {
                if (cells[rand].both_area[0] < cells[rand].both_area[1])
                {
                    //move
                    cells[rand].partition = 0;
                    tot_area[0] += cells[rand].both_area[0];
                    tot_area[1] -= cells[rand].both_area[1];
                    continue;
                }
            }
            else if (tot_area[0] > areaconstraint[0] && cells[rand].partition == 1) {
                continue;
            }
            else if (tot_area[0] > areaconstraint[0] && cells[rand].both_area[1] < cells[rand].both_area[0]) {
                cells[rand].partition = 1;
                tot_area[0] -= cells[rand].both_area[0];
                tot_area[1] += cells[rand].both_area[1];
                continue;
            }
            else if (tot_area[0] > areaconstraint[0] && tot_area[1] + cells[rand].both_area[1] < areaconstraint[1]) {
                cells[rand].partition = 1;
                tot_area[0] -= cells[rand].both_area[0];
                tot_area[1] += cells[rand].both_area[1];
                continue;
            }


            if (tot_area[1] > areaconstraint[1] && tot_area[0] > areaconstraint[0] && cells[rand].partition == 0) {

                if (cells[rand].both_area[1] < cells[rand].both_area[0])
                {
                    //move
                    cells[rand].partition = 1;
                    tot_area[1] += cells[rand].both_area[1];
                    tot_area[0] -= cells[rand].both_area[0];
                    continue;
                }
            }
            else if (tot_area[1] > areaconstraint[1] && cells[rand].partition == 0) {
                continue;
            }
            else if (tot_area[1] > areaconstraint[1] && cells[rand].both_area[0] < cells[rand].both_area[1]) {
                cells[rand].partition = 0;
                tot_area[1] -= cells[rand].both_area[1];
                tot_area[0] += cells[rand].both_area[0];
                continue;
            }
            else if (tot_area[1] > areaconstraint[1] && tot_area[0] + cells[rand].both_area[0] < areaconstraint[0]) {
                cells[rand].partition = 0;
                tot_area[1] -= cells[rand].both_area[1];
                tot_area[0] += cells[rand].both_area[0];
                continue;
            }
        }
        tot_cutsize(cells, nets);
        if (min_cut_size > tot_cut_size) {
            min_cut_size = tot_cut_size;
            temp_cells = cells;
        }
        tot_area[0] = 0;
        tot_area[1] = 0;
    }

    output_file_verify(temp_cells, dieAssignments, outputFile_verify, min_cut_size);
    std::cout << "after move vertex to fulfill the objective cutsize: " << min_cut_size << endl;
    std::cout << "fulfill the objective" << endl;
}



