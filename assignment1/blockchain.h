#include<bits/stdc++.h>
#include "event.h"
#include "block.h"
#include "general.h"
using namespace std;

// transaction format
// TxnID: IDx pays IDy C coins
// TxnID:IDk mines 50 coins

general_global_variables * gv;

map<pair<int,int>, double> propagation_delay;

//Exponential distribution related variables
random_device rd_slow_lat, rd_fast_lat;
mt19937 *gen_slow_lat, *gen_fast_lat;
exponential_distribution<double> *d_slow_lat, *d_fast_lat;

random_device rd_slow_mining, rd_fast_mining;
mt19937 *gen_slow_mining, *gen_fast_mining;
exponential_distribution<double> *d_slow_mining, *d_fast_mining;


typedef pair<double, struct event> event_t;

class compare
{
public:
    bool operator()(const event_t& lhs, const event_t& rhs)
    {
        return rhs.first < lhs.first;
    }

};
priority_queue<event_t, vector<event_t>, compare > event_queue;

// helper function declarations   
vector<vector<int>> parse_arg(int n, char** paramenters);
int find_transaction_id(string trans);
double find_amount(string trans);
int find_sender_id(string trans);
int find_reciever_id(string trans);