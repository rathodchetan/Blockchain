#include<bits/stdc++.h>
#include "node.cpp"
using namespace std;

class node **nodes;

void create_network(int adv_per_con){
    while(true){
        map<int,vector<int>> adjacency_list;
        vector<int> no_of_peers(gv->node_count,0);
        vector<int> available_nodes;   // nodes which have less than 8 peers
        map< pair<int,int> , bool> edge; // pair of nodes which have a edge
        for(int i=2; i<=gv->node_count; i++){
            available_nodes.push_back(i);
        }
        bool not_p = false;
        for(int i=1; i<=gv->node_count; i++){
            if(no_of_peers[i-1] <4){
                int r = 4 + rand()%5;
                r = r - no_of_peers[i-1];
                vector<int> a_av_n; // peers which have less than 8 
                for(auto e: available_nodes){
                    if(!edge[{i, e}]){
                        a_av_n.push_back(e);
                    }
                }
                if(a_av_n.size() < r) { // if the no of free node is less than required node
                    not_p = true;
                    break;
                }
                set <int> con;
                while(con.size() != r){
                    con.insert(rand()%a_av_n.size());
                }
                for(auto c: con){
                    edge[{i,a_av_n[c]}] = true;
                    edge[{a_av_n[c],i}] = true;
                    adjacency_list[i].push_back(a_av_n[c]);
                    adjacency_list[a_av_n[c]].push_back(i);
                    no_of_peers[i-1]++;
                    no_of_peers[a_av_n[c]-1]++;
                }
            }
            vector<int> temp;
            for(auto c  : available_nodes){
                if(c == i+1) continue;
                if(no_of_peers[c-1] < 8){
                    temp.push_back(c);
                }
            }
            if(no_of_peers[i-1] <8) temp.push_back(i);
            available_nodes.clear();
            available_nodes = temp;
        }
        // if(not_p){ cout << "not posible combination "<< endl; continue;}

        // checking if the graph is connected 
        vector<bool> visited(gv->node_count, false);
        stack<int> stack;
        stack.push(1);
        while (!stack.empty())
        {
            // Pop a vertex from stack and print it
            int s = stack.top();
            stack.pop();
            if (!visited[s-1])
            {
                visited[s-1] = true;
            }
            for (auto i : adjacency_list[s])
                if (!visited[i-1])
                    stack.push(i);
        }
        for(int i =2; i<=gv->node_count; i++){
            if(!visited[i-1]) {cout << "graph is not connected trying again! please wait"<< endl;}
        }
        set<int> peers_adj;
        for(auto i : adjacency_list[1]){
            peers_adj.insert(i);
        }
        while(adv_per_con > peers_adj.size()){
            int n = rand()%gv->node_count + 1;
            if(n !=1)
            peers_adj.insert(n);
        }
        // cout << "Properly connected graph has been formed" << endl;
        // cout << "Now its the time to simulate"<< endl;
        for(auto i : peers_adj){
            adjacency_list[i].push_back(1);
            adjacency_list[1].push_back(i);
        }
        for(int i=1; i<=gv->node_count; i++){
            nodes[i-1]->set_peers(adjacency_list[i]);
        }
        break;
    }
    return;
}

void simulate(){
    // to start the simulation we are creating events for mining of the genesis block for all the nodes and the events for a transaction 
    // for every node. Then these event will trigger the future events and it will run till infinity if we will not give some conditon
    // to break 
    for(int i=0; i<gv->node_count; i++){
        nodes[i]->create_genesis_block();
    }
    for(int i=0; i<gv->node_count; i++){
        nodes[i]->create_next_transaction();
    } 
    // cout << "Starting the simulation ---------------------"<< endl;
    bool break_sim = false;
    while(!event_queue.empty()){
        event_t t = event_queue.top();
        event_queue.pop();
        event tm = t.second;
        gv->current_time = t.first;


        if(tm.event_t == create_transaction and !break_sim){ // geting a event for creating a transaction
            int sender = tm.from;
            nodes[sender-1]->recieved_transaction(tm);
            nodes[sender-1]->create_next_transaction();
        }
        else if(tm.event_t == recieve_transaction and !break_sim){ // geting a event for listening transaction from some peer
            int sender = tm.intended_to;
            nodes[sender-1]->recieved_transaction(tm);
        }
        else if(tm.event_t == block_mined and !break_sim){ // getting a event for completion of mining of a block 
            int sender = tm.from;
            if (tm.mined_block->chain_length == 31) break_sim = true;
            if (!nodes[sender-1]->get_last_block_in_longest_chain()){
                // cout <<"block no " << tm.mined_block->block_id << " with "<< tm.mined_block->transactions.size() << " transaction mined by "<<tm.from << endl;
                nodes[sender-1]->recieve_block(tm);
            }
            else if (nodes[sender-1]->get_last_block_in_longest_chain()->block_id == tm.mined_block->last_block){
                // cout <<"block no " << tm.mined_block->block_id << " with "<< tm.mined_block->transactions.size() << " transaction mined by "<<tm.from << endl;
                nodes[sender-1]->recieve_block(tm);
            }
        }
        else if(tm.event_t == block_recieved){ // getting a event for recieving a mined block from some peer
            int sender = tm.intended_to;
            nodes[sender-1]->recieve_block(tm);
        }
    }
    // cout << "Ending the simulation ------------------------"<< endl;
    // finally executing some helper functions to make blockchain tree for every node and to get some additonal info about chains 
    for(int i=0; i<gv->node_count; i++){
        nodes[i]->print_tree();
        nodes[i]->write_data_to_file(0);
    }

}

int main(int argc, char** argv){
    mining_power_of_adversary =10;
    mining_power_of_adversary =stoi(argv[3]);
    int no_of_adversary_node =1;
    // initialisation of global variables
    int n_c = 30;
    gv = new general_global_variables(n_c, 600, 600); // arg1- No of nodes in network, arg2 - mean inter-transaction time,
    // arg3 - mean inter block mining time
    nodes = new class node*[gv->node_count];
    gv->link_speed  = new network_speed[gv->node_count];

    // initialisation of exponential distributions for latencies of links
    gen_slow_lat = new mt19937(rd_slow_lat());
    gen_fast_lat = new mt19937(rd_fast_lat());
    d_slow_lat = new exponential_distribution<double>(1.0/19.2);
    d_fast_lat = new exponential_distribution<double>(1.0/0.96);
    
    // parsing command line arguments
    vector<vector<int>> features = parse_arg(argc,argv);

    // calculation of hasing powers
    gv->high_cpu_nodes = gv->node_count - gv->low_cpu_nodes-no_of_adversary_node;
    gv->mean_low_cpu_hashing_time = gv->mean_block_interarrival_time*(gv->low_cpu_nodes + 10*gv->high_cpu_nodes) * (100/(100-mining_power_of_adversary*no_of_adversary_node)) ;
    gv->mean_high_cpu_hashing_time = gv->mean_low_cpu_hashing_time/10;



    // initialisation of exponential distributions for block mining time
    gen_slow_mining = new mt19937(rd_slow_mining());
    gen_fast_mining = new mt19937(rd_fast_mining());
    gen_adversary_mining = new mt19937(rd_adversary_mining());
    d_slow_mining = new exponential_distribution<double>(1.0/gv->mean_low_cpu_hashing_time);
    d_fast_mining = new exponential_distribution<double>(1.0/gv->mean_high_cpu_hashing_time);
    // cout << gv->mean_low_cpu_hashing_time << " "<< gv->mean_high_cpu_hashing_time << " " << 1/(mining_power_of_adversary/(gv->mean_block_interarrival_time*100)) << endl;
    d_adversary_mining = new exponential_distribution<double>(mining_power_of_adversary/(gv->mean_block_interarrival_time*100));

    // initialising nodes
    for(int i=1; i<=gv->node_count; i++){
        gv->link_speed[i-1] = network_speed(features[0][i-1]);
        nodes[i-1] = new node(i==1, false, i ,network_speed(features[0][i-1]) ,cpu_power(features[1][i-1]), gv->mean_transaction_time);
    } 

    // initialising propagation delays for every pair of nodes
    for(int i=1; i<=gv->node_count; i++){
        for(int j=i+1; j<=gv->node_count; j++){
            propagation_delay[{i,j}] = 10 + rand()%491;
        }
    }

    create_network((stoi(argv[4])*n_c)/100);
    simulate();
}

