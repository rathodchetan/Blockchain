#include <bits/stdc++.h>
#include "blockchain.h"
using namespace std;


// function to parse command line argument and then select z0 and z1 % random node for slow and fast links and cpus respectively
vector<vector<int>> parse_arg(int n, char** paramenters){
    set<int> Z0, Z1;
    int z0p = (atoi(paramenters[1]) * gv->node_count)/100;
    int z1p = (atoi(paramenters[2]) * gv->node_count)/100;
    vector<vector<int>> ret(2, vector<int>(gv->node_count,0));
    int i=4;
    while(Z0.size() != z0p){
        Z0.insert(rand()%gv->node_count +1);
    }
    while(Z1.size() != z1p){
        Z1.insert(rand()%gv->node_count +1);
    }
    deque<int> z0,z1;
    for(auto i: Z0){
        z0.push_back(i);
    }
    for(auto i: Z1){
        z1.push_back(i);
    }

    for(i=0; i<gv->node_count; i++){
        if(!z0.empty()) {
            if(z0.front()==i+1){z0.pop_front();}
            else { ret[0][i] = 1;}
        }
        else ret[0][i] = 1;
        if(!z1.empty()) {
            if(z1.front()==i+1){z1.pop_front(); gv->low_cpu_nodes++;}
            else { ret[1][i] = 1;}
        }
        else ret[1][i] = 1;
    }
    return ret;
}

int find_transaction_id(string trans){
    string tid = trans.substr(0, trans.find(":")); 
    return stoi(tid);
}

double find_amount(string trans){
    string amount;
    if(trans.find("mines") < trans.size()){
        amount = trans.substr(trans.find("mines")+6, trans.find("coins")-2 -(trans.find("mines")+5) );
    }   
    else {
        amount = trans.substr(trans.find("pays"));
        amount = amount.substr(amount.find(" ")+1);
        amount = amount.substr(amount.find(" ")+1);
        amount = amount.substr(0, amount.find(" "));
    }
    return stod(amount);
}
int find_sender_id(string trans){
    string sender_id;
    sender_id = trans.substr(trans.find(":")+1, trans.find(" ")-1 -(trans.find(":")) );
    return stoi(sender_id);
}
int find_reciever_id(string trans){
    string reciver_id;
    if(trans.find("mines") < trans.size()){
        reciver_id = trans.substr(trans.find(":")+1, trans.find(" ")-1 -(trans.find(":")) );
    }   
    else {
        reciver_id = trans.substr(trans.find("pays")+5);
        reciver_id = reciver_id.substr(reciver_id.find(":")+1, reciver_id.find(" ")-1 -(reciver_id.find(":")));
    }
    return stoi(reciver_id);
}