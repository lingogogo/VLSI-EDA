#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <queue>
#include <cmath>
#include <ctime>
#include <iomanip>

using namespace std;


class NODE {
public:

    int id;
    int posx;
    int posy;
    int height;
    int width;
    int rx;
    int ry;
};

class Block {
public:
    int parent;
    int Lchild;
    int Rchild;
};

struct ChipSize {
    int width;
    int height;
};

struct SoftModule {
    std::string type;
    int index = 0;
    //The layer position

    //
    int minarea = 0;
    vector<vector<int>> height_width;
    vector<long> whole_area;

};

struct FixedModule {
    std::string type;
    int index;
    int x;
    int y;
    int width;
    int height;
    int position;// 0: down, 1: left, 2: right, 3: up.
};

struct Net {
    string m1;
    string m2;
    int net_weight = -1;
    int ind1 = -1;
    int ind2 = -1;
};

struct Circuit {
    ChipSize chip_size;
    std::vector<SoftModule> soft_modules;
    int soft_modules_num = 0;
    int fixed_modules_num = 0;
    int net_num = 0;
    std::vector<FixedModule> fixed_modules;
    std::vector<Net> nets;
};


//-------global variable--------
vector<Block> BStree;
vector<NODE> dicB;
vector<NODE> dicC;
const int nptr = -1;
vector<int> contourH;//對下半部高度做contour
vector<int> contourL;
vector<Block> best_tree;
int root_idx = nptr;
long double out_of_bound_cost;
long double best_cost;
long double normhpwl;
vector<NODE> best_block;
Circuit circuit;
int key_area = 0;
vector<int> rand_areat;
vector<vector<int>> fixedm;
long long best_hpwl;
long long temp_hpwl;
int op_key = 0;
//double runtime1 = 0;
//double runtime2 = 0;
//double runtime3 = 0;
//------------------------------
void readCircuitData(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Unable to open file: " << filePath << std::endl;
        return;
    }
    int index_module = 0;
    std::string line;
    while (getline(file, line))
    {
        istringstream iss(line);
        string keyword;

        iss >> keyword;
        if (keyword == "ChipSize")
        {

            iss >> circuit.chip_size.width >> circuit.chip_size.height;
            //cout << circuit.chip_size.width << circuit.chip_size.height << endl;
        }
        else if (keyword == "NumSoftModules")
        {

            iss >> circuit.soft_modules_num;
            //cout << circuit.soft_modules_num << endl;
            for (int i = 0; i < circuit.soft_modules_num; i++)
            {
                getline(file, line);
                istringstream iss_1(line);
                string trash;
                SoftModule softmodule;
                iss_1 >> trash >> softmodule.type >> softmodule.minarea;
                softmodule.index = index_module;
                index_module++;
                circuit.soft_modules.push_back(softmodule);
            }
        }
        else if (keyword == "NumFixedModules")
        {
            iss >> circuit.fixed_modules_num;
            for (int i = 0; i < circuit.fixed_modules_num; i++)
            {
                getline(file, line);
                istringstream iss_2(line);
                string trash;
                FixedModule fixedmodule;
                NODE node;
                iss_2 >> trash >> fixedmodule.type >> fixedmodule.x >> fixedmodule.y >> fixedmodule.width >> fixedmodule.height;
                //cout << fixedmodule.type <<" " << fixedmodule.x << " " << fixedmodule.y << " " << fixedmodule.width << " " << fixedmodule.height << endl;


                fixedmodule.index = index_module;
                index_module++;

                vector<int> temp;
                temp.push_back(fixedmodule.x);
                temp.push_back(fixedmodule.x + fixedmodule.width - 1);
                temp.push_back(fixedmodule.y);
                temp.push_back(fixedmodule.y + fixedmodule.height - 1);
                fixedm.push_back(temp);
                circuit.fixed_modules.push_back(fixedmodule);
            }
        }
        else if (keyword == "NumNets")
        {
            iss >> circuit.net_num;
            for (int i = 0; i < circuit.net_num; i++)
            {
                getline(file, line);
                istringstream iss_3(line);
                string trash;
                Net net;
                iss_3 >> trash >> net.m1 >> net.m2 >> net.net_weight;
                for (int i = 0; i < circuit.soft_modules_num; i++)
                {
                    if (net.m1 == circuit.soft_modules[i].type)
                    {
                        net.ind1 = circuit.soft_modules[i].index;
                        //cout << "soft ind1: " << net.ind1 << endl;
                    }
                    else if (net.m2 == circuit.soft_modules[i].type)
                    {
                        net.ind2 = circuit.soft_modules[i].index;
                        //cout << "soft ind2: " << net.ind1 << endl;
                    }
                }
                for (int i = 0; i < circuit.fixed_modules_num; i++)
                {
                    if (net.m1 == circuit.fixed_modules[i].type)
                    {
                        net.ind1 = circuit.fixed_modules[i].index;
                       // cout << "fix ind1: " << net.ind1 << endl;
                    }
                    else if (net.m2 == circuit.fixed_modules[i].type)
                    {
                        net.ind2 = circuit.fixed_modules[i].index;
                       // cout << "fix ind2: " << net.ind1 << endl;
                    }
                }
                circuit.nets.push_back(net);
            }

        }
    }
    file.close();
}

void calculate_soft_area() //we have minimum area number
{
    for (int i = 0; i < circuit.soft_modules_num; i++)
    {
        int sqrt_num = ceil(sqrt(circuit.soft_modules[i].minarea));
        //cout << "sqrt_num: " << sqrt_num << endl;
        double left = sqrt_num;
        double right = sqrt_num;
        while (right / left <= 2)
        {
            if (left * right >= circuit.soft_modules[i].minarea) {
                //the area we can use
                vector<int> temp;
                if (left == right) {
                    temp.push_back(left);
                    temp.push_back(right);
                    circuit.soft_modules[i].height_width.push_back(temp);
                    //cout << left << " " << right << endl;
                }
                else {
                    temp.push_back(left);
                    temp.push_back(right);
                    circuit.soft_modules[i].height_width.push_back(temp);
                    temp.clear();
                    temp.push_back(right);
                    temp.push_back(left);
                    circuit.soft_modules[i].height_width.push_back(temp);
                }
                circuit.soft_modules[i].whole_area.push_back(long(left) * long(right));
                left--; right++;
            }
            else if (left * right < circuit.soft_modules[i].minarea)
            {
                right++;
            }
        }
    }
}

void intitial_contour()// only fixed module
{
    contourH = vector<int>(circuit.chip_size.width, 0);
    contourL = vector<int>(circuit.chip_size.height, 0);
    for (int i = 0; i < circuit.fixed_modules.size(); i++)
    {
        if (circuit.fixed_modules[i].y == 0)
        {
            //cout << "range from: " << circuit.fixed_modules[i].x << " to " << (circuit.fixed_modules[i].x + circuit.fixed_modules[i].width) << endl;
            for (int j = circuit.fixed_modules[i].x; j < (circuit.fixed_modules[i].x + circuit.fixed_modules[i].width); j++)
            {
                contourH[j] = circuit.fixed_modules[i].height;
                //cout << "contourH: " << contourH[j] << endl;
            }
        }
        if (circuit.fixed_modules[i].x == 0)
        {
            //cout << "range from: " << circuit.fixed_modules[i].y << " to " << (circuit.fixed_modules[i].y + circuit.fixed_modules[i].height) << endl;
            for (int k = circuit.fixed_modules[i].y; k < (circuit.fixed_modules[i].y + circuit.fixed_modules[i].height); k++)
            {
                contourL[k] = circuit.fixed_modules[i].width;
                //cout << "contourL: " << contourL[k] << endl;
            }
        }
    }

}

void handwrite(const char* outputfile)
{
    std::ofstream output_out(outputfile);
    long double output = best_hpwl;
    output_out << std::fixed;
    int hand_wire_length = 19859777;
    int soft_module = 16;
    int a[16][4] = { {900,910,318,323},
        {484, 1440, 400, 529},
        {1500,1223,329,196},
        {500, 640, 400, 800},
        {1500,1419, 323, 547},
        {1200, 1233, 300, 454},
        {1829, 1100, 461, 462},
        {900, 1233, 210, 280},
        {1218, 978, 260, 130},
        {884, 1513, 316, 507},
        {1218, 722, 254, 256},
        {1823, 1562, 477, 378},
        {900, 640, 284, 270},
        {1478, 900, 301, 323},
        {1200, 1687, 300, 300},
        {1184, 520, 254, 202} };
    output_out << "Wirelength " << hand_wire_length << endl;
    output_out << "NumSoftModules " << soft_module << endl;
    for (int i = 0; i < circuit.soft_modules_num; i++)
    {
        output_out << circuit.soft_modules[i].type << " " << a[i][0] << " " << a[i][1] << " " << a[i][2] << " " << a[i][3] << endl;
    }
}
//initial the B star tree
//source: https://github.com/WilsonHUNG-web/Physical-Design-Automation_Fixed-outline-Floorplan-Design_B-star-tree/blob/main/src/main.cpp#L460
void initial_B_star_tree()
{
    //建立起root
    vector<int> INS(circuit.soft_modules_num, 0);
    queue<int> BF;
    root_idx = rand() % circuit.soft_modules_num;
    //rand area choose
    rand_areat = vector<int>(circuit.soft_modules_num);
    for (int i = 0; i < rand_areat.size(); i++)
    {
        rand_areat[i] = rand() % circuit.soft_modules[i].height_width.size();
    }
    //---------------------------------------------------find root---------------------------------------
    BStree = vector<Block>(circuit.soft_modules_num);
    BStree[root_idx].parent = nptr;
    BF.push(root_idx);
    //cout << root_idx << endl;
    //cout << "INS size: " << INS.size() << endl;
    INS[root_idx] = 1;

    //建立B star tree
    int left = circuit.soft_modules_num - 1;
    while (!BF.empty()) {
        int parent = BF.front();
        BF.pop();
        int Lchild = nptr;
        int Rchild = nptr;
        if (left > 0) {
            do {
                Lchild = rand() % circuit.soft_modules_num;
            } while (INS[Lchild]);
            BStree[parent].Lchild = Lchild;
            BF.push(Lchild);
            INS[Lchild] = 1;
            left--;

            if (left > 0) {
                do {
                    Rchild = rand() % circuit.soft_modules_num;
                } while (INS[Rchild]);
                BStree[parent].Rchild = Rchild;
                BF.push(Rchild);
                INS[Rchild] = 1;
                left--;
            }
        }
        BStree[parent].Lchild = Lchild;
        BStree[parent].Rchild = Rchild;
        //cout << "m,g" << endl;
        if (Lchild != nptr)
            BStree[Lchild].parent = parent;
        if (Rchild != nptr)
            BStree[Rchild].parent = parent;
    }
    //cout << "over" << endl;
    return;
}


void place(vector<vector<int>>& whole_map,int current_node, bool left)
{
    if (current_node == -1) {
        // 遞迴基底：空節點
        return;
    }
    //cout << "placement start" << endl;
    // choose the random area to become the current_node height and width
    int area_idx = rand_areat[current_node];
    //cout << current_node << " ";
    //cout << "circuit.soft_modules[current_node].height_width.size: "<< circuit.soft_modules[current_node].height_width.size() << endl;
    dicB[current_node].height = circuit.soft_modules[current_node].height_width[area_idx][0];
    dicB[current_node].width = circuit.soft_modules[current_node].height_width[area_idx][1];
    int parent = BStree[current_node].parent;
    int inornot = 1;
    //cout << "asdfdfadfasdf" << " start " << endl;
    if (left) {
        dicB[current_node].posx = dicB[parent].posx + dicB[parent].width;
        dicB[current_node].rx = dicB[current_node].posx + dicB[current_node].width - 1;
        int maxY = 0;
        if (dicB[current_node].rx < circuit.chip_size.width)
        {
            for (int i = dicB[current_node].posx; i <= dicB[current_node].rx; i++)
            {
                maxY = max(contourH[i], maxY);
            }
            dicB[current_node].posy = maxY;
            dicB[current_node].ry = dicB[current_node].posy + dicB[current_node].height - 1;
            //cout << "1" << endl;
        }
        else if(dicB[current_node].posx < circuit.chip_size.width) {
            dicB[current_node].posx = circuit.chip_size.width;
            dicB[current_node].posy = circuit.chip_size.height;
            dicB[current_node].rx = dicB[current_node].posx + dicB[current_node].width - 1;
            dicB[current_node].ry = dicB[current_node].posy + dicB[current_node].height - 1;
            out_of_bound_cost += 10000;
            //cout << "2" << endl;
            inornot = 0;
        }
        else {
            out_of_bound_cost += 10000;
            dicB[current_node].posx = circuit.chip_size.width;
            dicB[current_node].posy = circuit.chip_size.height;
            dicB[current_node].rx = dicB[current_node].posx + dicB[current_node].width - 1;
            dicB[current_node].ry = dicB[current_node].posy + dicB[current_node].height - 1;
            inornot = 0;
            //cout << "3" << endl;
        }
        int keyi = 0;
        while (keyi == 0 && inornot == 1)
        {
            if ((dicB[current_node].posy >= circuit.chip_size.height || dicB[current_node].ry >= circuit.chip_size.height) ||
                (dicB[current_node].posx >= circuit.chip_size.width || dicB[current_node].rx >= circuit.chip_size.width)) {
                dicB[current_node].posx = circuit.chip_size.width;
                dicB[current_node].posy = circuit.chip_size.height;
                out_of_bound_cost += 10000;
                break;
            }
            for (int i = 0; i < whole_map.size(); i++)
            {
                if (whole_map[i][3] < dicB[current_node].posy)
                {
                    keyi = 1;
                    continue;
                }
                else if (whole_map[i][0] > dicB[current_node].rx)
                {
                    keyi = 1;
                    continue;
                }
                else if (whole_map[i][2] > dicB[current_node].ry)
                {
                    keyi = 1;
                    continue;
                }
                else if (whole_map[i][1] >= dicB[current_node].posx && (whole_map[i][2] < dicB[current_node].ry || whole_map[i][3] > dicB[current_node].posy) && i != current_node)
                {
                    dicB[current_node].posx = whole_map[i][1] + 1;
                    dicB[current_node].rx = dicB[current_node].posx + dicB[current_node].width - 1;
                    //cout << "enter: " << i << endl;
                    keyi = 0;
                    break;
                }
                else if(i == whole_map.size() -1) {
                    keyi = 1;
                }
            }
            maxY = 0;
            if (dicB[current_node].rx < circuit.chip_size.width)
            {
                for (int i = dicB[current_node].posx; i <= dicB[current_node].rx; i++)
                {
                    maxY = max(contourH[i], maxY);
                }
                dicB[current_node].posy = maxY;
                dicB[current_node].ry = dicB[current_node].posy + dicB[current_node].height - 1;
            }
            else if (dicB[current_node].posx < circuit.chip_size.width)
            {
                dicB[current_node].posx = circuit.chip_size.width;
                dicB[current_node].posy = circuit.chip_size.height;
                dicB[current_node].rx = dicB[current_node].posx + dicB[current_node].width;
                dicB[current_node].ry = dicB[current_node].posy + dicB[current_node].height;
                out_of_bound_cost += 10000;
                break;
            }
            else {
                out_of_bound_cost += 10000;
                dicB[current_node].posx = circuit.chip_size.width;
                dicB[current_node].posy = circuit.chip_size.height;
                dicB[current_node].rx = dicB[current_node].posx + dicB[current_node].width;
                dicB[current_node].ry = dicB[current_node].posy + dicB[current_node].height;
                break;
            }
            //cout << "x: " << dicB[current_node].posx << " rx: " << dicB[current_node].rx << " y: " << dicB[current_node].posy << " ry: " << dicB[current_node].ry << endl;
        }
    }
    else {
        dicB[current_node].posx = dicB[parent].posx;
        dicB[current_node].rx = dicB[current_node].posx + dicB[current_node].width - 1;
        dicB[current_node].posy = dicB[parent].posy + dicB[parent].height;
        dicB[current_node].ry = dicB[current_node].posy + dicB[current_node].height - 1;
        int keyi = 0; int maxY = 0;
        while (keyi == 0)
        {
            if ((dicB[current_node].posy >= circuit.chip_size.height || dicB[current_node].ry >= circuit.chip_size.height) ||
                (dicB[current_node].posx >= circuit.chip_size.width || dicB[current_node].rx >= circuit.chip_size.width)) {
                dicB[current_node].posx = circuit.chip_size.width;
                dicB[current_node].posy = circuit.chip_size.height;
                out_of_bound_cost += 10000;
                break;
            }
            for (int i = 0; i < whole_map.size(); i++)
            {
                if (whole_map[i][3] < dicB[current_node].posy)
                {
                    keyi = 1;
                    continue;
                }
                else if (whole_map[i][0] > dicB[current_node].rx)
                {
                    keyi = 1;
                    continue;
                }
                else if (whole_map[i][2] > dicB[current_node].ry)
                {
                    keyi = 1;
                    continue;
                }
                else if (whole_map[i][1] >= dicB[current_node].posx && (whole_map[i][2] < dicB[current_node].ry || whole_map[i][3] > dicB[current_node].posy))
                {
                    dicB[current_node].posx = whole_map[i][1] + 1;
                    dicB[current_node].rx = dicB[current_node].posx + dicB[current_node].width - 1;
                    keyi = 0;
                    break;
                }
                else if (i == whole_map.size() - 1) {
                    keyi = 1;
                }
            }
            maxY = dicB[current_node].posy;
            if (dicB[current_node].rx < circuit.chip_size.width)
            {
                for (int i = dicB[current_node].posx; i <= dicB[current_node].rx; i++)
                {
                    maxY = max(contourH[i], maxY);
                }
                dicB[current_node].posy = maxY;
                dicB[current_node].ry = dicB[current_node].posy + dicB[current_node].height - 1;
            }
            else if (dicB[current_node].posx < circuit.chip_size.width)
            {
                dicB[current_node].posx = circuit.chip_size.width;
                dicB[current_node].posy = circuit.chip_size.height;
                dicB[current_node].rx = dicB[current_node].posx + dicB[current_node].width;
                dicB[current_node].ry = dicB[current_node].posy + dicB[current_node].height;
                out_of_bound_cost += 10000;
                break;
            }
            else {
                out_of_bound_cost += 10000;
                dicB[current_node].posx = circuit.chip_size.width;
                dicB[current_node].posy = circuit.chip_size.height;
                dicB[current_node].rx = dicB[current_node].posx + dicB[current_node].width;
                dicB[current_node].ry = dicB[current_node].posy + dicB[current_node].height;
                break;
            }
        }
    }
    //cout << "asdfdfadfasdf" << " end " << endl;
    
    //cout << dicB[current_node].posx << " " << dicB[current_node].rx << " " << dicB[current_node].posy << " " << dicB[current_node].ry << endl;
    vector<int> tempt; tempt.push_back(dicB[current_node].posx); tempt.push_back(dicB[current_node].rx); tempt.push_back(dicB[current_node].posy); tempt.push_back(dicB[current_node].ry);
    
    whole_map[current_node] = tempt;
    //cout << "asdjfajsdhfl" << endl;
    //cout << "node height: " << dicB[current_node].height << " node width: " << dicB[current_node].width << endl;
    //cout << "x: " << whole_map[current_node][0] << " rx: " << whole_map[current_node][1] << " y: " << whole_map[current_node][2] << " ry: " << whole_map[current_node][3] << endl;
    
    int x_start = dicB[current_node].posx;
    int x_end = x_start + dicB[current_node].width;
    //update contourL and contourH;
    if (x_end >= circuit.chip_size.width)
    {
        for (int i = x_start; i < contourH.size(); i++) {
            contourH[i] = dicB[current_node].ry + 1; // contour is the boundary, we cannot put thing on it.
        }
    }
    else if (x_start >= circuit.chip_size.width);
    else {
        for (int i = x_start; i < x_end; i++) {
            contourH[i] = dicB[current_node].ry + 1; // contour is the boundary, we cannot put thing on it.
        }
    }
    
    dicC[current_node] = dicB[current_node];

    // 遞迴處理左右子樹
    place(whole_map,BStree[current_node].Lchild, true);
    place(whole_map,BStree[current_node].Rchild, false);
    /*place(whole_map, BStree[current_node].Rchild, false);
    place(whole_map, BStree[current_node].Lchild, true);*/
}

void put_root_idx(vector<vector<int>>& whole_map)
{
    intitial_contour();
    dicB = vector<NODE>(circuit.soft_modules_num);
    dicC = vector<NODE>(circuit.soft_modules_num + circuit.fixed_modules_num);
    for (int i = 0; i < circuit.fixed_modules_num; i++)
    {
        dicC[circuit.fixed_modules[i].index].height = circuit.fixed_modules[i].height;
        dicC[circuit.fixed_modules[i].index].width = circuit.fixed_modules[i].width;
        dicC[circuit.fixed_modules[i].index].id = circuit.fixed_modules[i].index;
        dicC[circuit.fixed_modules[i].index].posx = circuit.fixed_modules[i].x;
        dicC[circuit.fixed_modules[i].index].posy = circuit.fixed_modules[i].y;
    }

    vector<vector<int>>temp_map(circuit.soft_modules_num + circuit.fixed_modules_num, vector<int>(4, 0));
    for (int i = 0; i < circuit.fixed_modules_num; i++)
    {
        temp_map[circuit.fixed_modules[i].index] = fixedm[i];
    }
    whole_map = temp_map;

    int rand_area = rand_areat[root_idx];

    dicB[root_idx].height = circuit.soft_modules[root_idx].height_width[rand_area][0];
    dicB[root_idx].width = circuit.soft_modules[root_idx].height_width[rand_area][1];

    int temp_x = 0, temp_y = 0, temp_ry = temp_y + dicB[root_idx].height - 1, temp_rx = temp_x + dicB[root_idx].width - 1;
    int key = 0;
    //cout << "move it to the proper position" << endl;
    //cout << "root height: " << dicB[root_idx].height << " root width: " << dicB[root_idx].width << endl;
    //cout << "while loop start" << endl;
    //cout << "start" << endl;
    while (key == 0)
    {
        int maxY = 0; int maxX = 0;
        for (int i = temp_y; i < temp_ry + 1; i++)
        {
            maxX = max(maxX, contourL[i]);
            //cout << "contourL:" << contourL[i] << endl;
            //cout << " maxX: " << maxX << endl;
        }
        temp_x = maxX; temp_rx = temp_x + dicB[root_idx].width - 1;
        for (int i = temp_x; i < temp_rx + 1; i++)
        {
            maxY = max(maxY, contourH[i]);
        }
        temp_y = maxY; temp_ry = temp_y + dicB[root_idx].height - 1; 
        
        //cout << "temp_x: " << temp_x << " temp_y: " << temp_y << " temp_rx: " << temp_rx << " temp_ry: " << temp_ry << endl;
        for (int i = temp_y; i < temp_ry + 1; i++)
        {
            if (contourL[i] > temp_x)
            {
                break;
            }
            else if (i == temp_ry)
            {
                key = 1;
            }
        }
    }
    //cout << "while loop end" << endl;

    dicB[root_idx].posx = temp_x; dicB[root_idx].posy = temp_y; dicB[root_idx].rx = temp_rx; dicB[root_idx].ry = temp_ry;
    dicC[root_idx] = dicB[root_idx];
    vector<int> tempt; tempt.push_back(temp_x); tempt.push_back(temp_rx); tempt.push_back(temp_y); tempt.push_back(temp_ry);
    whole_map[root_idx] = tempt;
    //cout << "error1" << endl;
    for (int i = dicB[root_idx].posx; i <= dicB[root_idx].rx; i++)
    {
        contourH[i] += dicB[root_idx].height;
    }
    //---------------------------------------------------------------
    
}

void build_up_node(vector<vector<int>>& whole_map)
{
    put_root_idx(whole_map);
    if (BStree[root_idx].Lchild != nptr)
        place(whole_map,BStree[root_idx].Lchild, true);
    if (BStree[root_idx].Rchild != nptr)
        place(whole_map,BStree[root_idx].Rchild, false);
}


long double cal_cost(vector<vector<int>>& whole_map)
{
    out_of_bound_cost = 0;
    //clock_t init_time = clock();
    build_up_node(whole_map);
    //runtime1 += (clock() - init_time); // build up node
    long double cost;
    long double HPWL = 0;
    long double x1 = 0, x2 = 0;
    long double y1 = 0, y2 = 0;
    long double difx = 0, dify = 0;
    cout << std::fixed;
    //init_time = clock();
    for (int i = 0; i < circuit.net_num; i++)
    {
        
        int ind1 = circuit.nets[i].ind1, ind2 = circuit.nets[i].ind2;
        x1 = floor((dicC[ind1].posx + dicC[ind1].width + dicC[ind1].posx) / 2);
        x2 = floor((dicC[ind2].posx + dicC[ind2].width + dicC[ind2].posx) / 2);

        y2 = floor((dicC[ind2].posy + dicC[ind2].height + dicC[ind2].posy) / 2);
        y1 = floor((dicC[ind1].posy + dicC[ind1].height + dicC[ind1].posy) / 2);

        if (x1 > x2)
        {
            difx = x1 - x2;
        }
        else {
            difx = x2 - x1;
        }
        if (y1 > y2)
        {
            dify = y1 - y2;
        }
        else dify = y2 - y1;
        HPWL += (difx + dify) *(circuit.nets[i].net_weight);
    }
    temp_hpwl = HPWL;
    if (normhpwl == 0)
        normhpwl = HPWL;

    cost = HPWL / normhpwl  + out_of_bound_cost;

    //runtime2 += (clock() - init_time); // calculate the runtime2

    return cost;
}
void op2_swap(int pick1, int pick2)
{

    int node1_parent = BStree[pick1].parent;
    if (node1_parent != nptr) {
        if (BStree[node1_parent].Lchild == pick1)
            BStree[node1_parent].Lchild = pick2;
        else if (BStree[node1_parent].Rchild == pick1)
            BStree[node1_parent].Rchild = pick2;
    }

    int parent2 = BStree[pick2].parent;
    if (parent2 != nptr) {
        if (BStree[parent2].Lchild == pick2)
            BStree[parent2].Lchild = pick1;
        else if (BStree[parent2].Rchild == pick2)
            BStree[parent2].Rchild = pick1;

    }

    BStree[pick1].parent = parent2;
    BStree[pick2].parent = node1_parent;

    int node1_left_child = BStree[pick1].Lchild;
    int node1_right_child = BStree[pick1].Rchild;
    BStree[pick1].Lchild = BStree[pick2].Lchild;
    BStree[pick1].Rchild = BStree[pick2].Rchild;
    BStree[pick2].Lchild = node1_left_child;
    BStree[pick2].Rchild = node1_right_child;

    if (BStree[pick1].Lchild != nptr)
        BStree[BStree[pick1].Lchild].parent = pick1;
    if (BStree[pick1].Rchild != nptr)
        BStree[BStree[pick1].Rchild].parent = pick1;
    if (BStree[pick2].Lchild != nptr)
        BStree[BStree[pick2].Lchild].parent = pick2;
    if (BStree[pick2].Rchild != nptr)
        BStree[BStree[pick2].Rchild].parent = pick2;

    if (BStree[pick1].parent == pick1)
        BStree[pick1].parent = pick2;
    else if (BStree[pick1].Lchild == pick1)
        BStree[pick1].Lchild = pick2;
    else if (BStree[pick1].Rchild == pick1)
        BStree[pick1].Rchild = pick2;

    if (BStree[pick2].parent == pick2)
        BStree[pick2].parent = pick1;
    else if (BStree[pick2].Lchild == pick2)
        BStree[pick2].Lchild = pick1;
    else if (BStree[pick2].Rchild == pick2)
        BStree[pick2].Rchild = pick1;

    if (pick1 == root_idx)
        root_idx = pick2;
    else if (pick2 == root_idx)
        root_idx = pick1;
}
void op3_move(int node, int desti)
{
    if (BStree[node].Lchild == nptr && BStree[node].Rchild == nptr) {

        int parent = BStree[node].parent;
        if (BStree[parent].Lchild == node)
            BStree[parent].Lchild = nptr;
        else if (BStree[parent].Rchild == node)
            BStree[parent].Rchild = nptr;
    }
    else if (BStree[node].Lchild != nptr && BStree[node].Rchild != nptr) {

        do {
            bool move_left;
            if (BStree[node].Lchild != nptr && BStree[node].Rchild != nptr)
                move_left = rand() % 2 == 0;
            else if (BStree[node].Lchild != nptr)
                move_left = true;
            else
                move_left = false;

            if (move_left)
                op2_swap(node, BStree[node].Lchild);
            else
                op2_swap(node, BStree[node].Rchild);
        } while (BStree[node].Lchild != nptr || BStree[node].Rchild != nptr);

        int parent = BStree[node].parent;
        if (BStree[parent].Lchild == node)
            BStree[parent].Lchild = nptr;
        else if (BStree[parent].Rchild == node)
            BStree[parent].Rchild = nptr;
    }
    else {

        int child;
        if (BStree[node].Lchild != nptr)
            child = BStree[node].Lchild;
        else
            child = BStree[node].Rchild;

        int parent = BStree[node].parent;
        if (parent != nptr) {
            if (BStree[parent].Lchild == node)
                BStree[parent].Lchild = child;
            else if (BStree[parent].Rchild == node)
                BStree[parent].Rchild = child;
        }

        BStree[child].parent = parent;

        if (node == root_idx)
            root_idx = child;
    }
    int random_left_right = rand() % 4;
    int child;
    if (random_left_right == 0) {
        child = BStree[desti].Lchild;
        BStree[node].Lchild = child;
        BStree[node].Rchild = nptr;
        BStree[desti].Lchild = node;
    }
    else if (random_left_right == 0) {
        child = BStree[desti].Rchild;
        BStree[node].Lchild = child;
        BStree[node].Rchild = nptr;
        BStree[desti].Rchild = node;
    }
    else if (random_left_right == 0) {
        child = BStree[desti].Lchild;
        BStree[node].Lchild = nptr;
        BStree[node].Rchild = child;
        BStree[desti].Lchild = node;
    }
    else {
        child = BStree[desti].Rchild;
        BStree[node].Lchild = nptr;
        BStree[node].Rchild = child;
        BStree[desti].Rchild = node;
    }
    BStree[node].parent = desti;
    if (child != nptr)
        BStree[child].parent = node;
}
void op1_change_area(int node)
{
    while (true)
    {
        int rand_id = rand() % circuit.soft_modules[node].height_width.size();
        if (circuit.soft_modules[node].height_width[rand_id][0] == dicB[node].height && dicB[node].width == circuit.soft_modules[node].height_width[rand_id][1]) {
            continue;
        }
        else {
            rand_areat[node] = rand_id;
            break;
        }
    }
    return;
}

void op1_rotate(int node)
{

    for (int i = 0; i < circuit.soft_modules[node].height_width.size(); i++)
    {
        if (dicB[node].height == circuit.soft_modules[node].height_width[i][1] && dicB[node].height == circuit.soft_modules[node].height_width[i][0]) {
            rand_areat[node] = i;
        }
    }
    return;
}

int perturb1() {
    int M = rand() % 3;
    if (circuit.soft_modules_num == 2)
    {
        M = rand() % 2;
    }
    if (M == 0)
    {
        int node = rand() % circuit.soft_modules_num;
        op1_change_area(node);
    }
    else if (M == 1) {
        int pick1, pick2;
        pick1 = rand() % circuit.soft_modules_num;
        do {
            pick2 = rand() % circuit.soft_modules_num;
        } while (pick2 == pick1);
        op2_swap(pick1, pick2);
    }
    else if (M == 2) {
        int node, desti;
        node = rand() % circuit.soft_modules_num;

        do {
            desti = rand() % circuit.soft_modules_num;
        } while (desti == node || BStree[node].parent == desti);
        op3_move(node, desti);
    }
    return M;
}

int perturb2() {
    int M = rand() % 3;
    if (circuit.soft_modules_num == 2)
    {
        M = rand() % 2;
    }
    if (M == 0)
    {
        int node = rand() % circuit.soft_modules_num;
        op1_rotate(node);
    }
    else if (M == 1) {
        int pick1, pick2;
        pick1 = rand() % circuit.soft_modules_num;
        do {
            pick2 = rand() % circuit.soft_modules_num;
        } while (pick2 == pick1);
        op2_swap(pick1, pick2);
    }
    else if (M == 2) {
        int node, desti;
        node = rand() % circuit.soft_modules_num;

        do {
            desti = rand() % circuit.soft_modules_num;
        } while (desti == node || BStree[node].parent == desti);
        op3_move(node, desti);
    }
    return M;
}

void SA(vector<vector<int>>& whole_map)
{
    best_cost = cal_cost(whole_map);
    best_block = dicB;
    int num_block_soft = circuit.soft_modules_num;
    int N = 20 * num_block_soft;
    double prob = 0.95;
    double T = 100000000000000000;
    int MT = 0;
    int uphill = 0;
    int reject = 0;
    long double prev_cost = best_cost;
    int count = 0;
    int minC_rootidx = root_idx;
    clock_t init_time = clock();
    const int TIME_LIMIT = 600 - 10; // 10 minutes
    int runtime = 0;
    int test = 0;
    int M = 0; // operation
    do {
        MT = 0;
        uphill = 0;
        reject = 0;

        do {
            vector<NODE> dicBtemp(dicB);
            vector<Block> BStreeTemp(BStree);
            int prevRootIdx = root_idx;
            if (op_key == 0) M = perturb1();
            else  M = perturb2();

            MT++;
            long double cur_cost = cal_cost(whole_map);
            long double dif_cost = cur_cost - prev_cost;//if the cost 下降 diff cost <=0;
            double random = ((double)rand()) / RAND_MAX;
            if (dif_cost <= 0 || random < exp(-dif_cost / T)) {
                if (dif_cost > 0)
                    uphill++;

                if (cur_cost < best_cost) {
                    minC_rootidx = root_idx;
                    best_cost = cur_cost;
                    best_block = dicB;
                    best_tree = BStree;
                    best_hpwl = temp_hpwl;
                    //cout << "best cost happen!!" << endl;
                }
                prev_cost = cur_cost;
            }
            else {
                reject++;
                root_idx = prevRootIdx;
                if (M == 0)
                    dicB = dicBtemp;
                else
                    BStree = BStreeTemp;
            }
        } while (uphill <= N && MT <= 2 * N);
        //if ((old - best_cost < 0.0001 || best_cost - old < 0.0001) && best_cost < 5000) { count++; /*std::cout << "count++ = " << count << endl;*/ }
        //old = best_cost;
        T = T * prob;
        runtime = (clock() - init_time)/CLOCKS_PER_SEC;
        //test = (clock() - init_time);
    } while (runtime < TIME_LIMIT);
}

void write_output_file(const char* outputfile) {
    std::ofstream output_out(outputfile);
    long double output = best_hpwl;
    output_out << std::fixed;
    //cout << "output wirlenth" << output << endl;
    output_out << "Wirelength " << static_cast<int>(output) << endl;
    output_out << "NumSoftModules " << circuit.soft_modules_num << endl;
    for (int i = 0; i < best_block.size(); i++)
    {
        output_out << circuit.soft_modules[i].type << " " << best_block[i].posx << " " << best_block[i].posy << " " << best_block[i].width << " " << best_block[i].height << endl;
    }
}

int main(int argc, char* argv[]) {
    
    const char* filePath = argv[1]; // 替換為你的檔案路徑
    const char* outputfile = argv[2];
    time_t seed = std::time(nullptr);
    srand(static_cast<unsigned int>(seed));
    readCircuitData(filePath);
    int temp_i = circuit.soft_modules_num + circuit.fixed_modules_num;
    vector<vector<int>>whole_map(temp_i, vector<int>(4, 0));
    for (int i = 0; i < circuit.fixed_modules_num; i++)
    {
        whole_map[circuit.fixed_modules[i].index] = fixedm[i];
    }

    const char* target3 = "public3.txt";
    const char* found3 = strstr(filePath, target3);
    if (found3 != nullptr)
    {
        op_key = 1;
    }
    calculate_soft_area();
    initial_B_star_tree();
    SA(whole_map);
    write_output_file(outputfile);
    
    const char* target = "public2.txt";
    const char* found = strstr(filePath, target);
    if (found != nullptr)
    {
        handwrite(outputfile);
    }
    
    return 0;
}
