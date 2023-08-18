#include<bits/stdc++.h>
#include "miner.cpp"
using namespace std;


class node : public miner {

    // connections related variables
    vector<int> peers;
    int peers_count;

    // variables to get exponential distribution for inter arrival transaction time
    random_device rd;
    mt19937 *gen;
    double mean_transaction_time;
    exponential_distribution<double> *d;
    map<struct block *, bool> block_is_in_tem_pool;
    // block related variables

    public:

    node(int id, network_speed s, cpu_power c, double time) : miner(id, c, s) {
        gen = new mt19937(rd());
        mean_transaction_time = time;
        d = new exponential_distribution<double>(1/mean_transaction_time);
    }
    // function to set peers of the node
    void set_peers(vector<int> p){
        peers_count = p.size();
        peers = p;
        return;
    }
    // function to create next transaction
    void create_next_transaction(){
        double time = gv->current_time + ((*d)(*gen));
        int payto=rand()%gv->node_count +1;
        while (id_m == payto){ // to get different reciver then himself
            payto=(rand()%gv->node_count) +1;
        }
        double amount_to_pay = amount_of_all_nodes_in_longest_chain[id_m-1]*double(0.01); // sending 1% of total amount  
        if(amount_to_pay == 0) return;
        string transaction = to_string(gv->get_transaction_id()) + ":" + to_string(id_m)+" pays " +to_string(payto) + " " +to_string(amount_to_pay)+ " coins";
        struct event e;
        e.from = id_m;
        e.transaction = transaction;
        e.event_t = create_transaction;
        event_queue.push({time, e}); // pushing the event into queue
        return;
    }
    void recieved_transaction(struct event e){
        if(transaction_already_listned[e.transaction]){ // to get loopless propagation of transactions
            return;
        }
        transaction_already_listned[e.transaction] = true;
        pending_transactions_in_longest_chain.push_back(e.transaction); // pushing the transaction into pending transactions
        for(int i=0; i<peers_count; i++){
            if(peers[i] == e.from) continue;
            struct event t =e;
            t.event_t = recieve_transaction;
            t.from = id_m;
            t.intended_to = peers[i];
            double time = calculate_latency(peers[i],1);
            event_queue.push({gv->current_time + time/1000.0, t}); // pushing the event into queue
        }
        return;
    }

    // when we are recieving a block from a peer this function will be called
    void recieve_block(struct event e){
        struct block *rec_block = e.mined_block;

        if(block_pointers[rec_block->block_id] != NULL){ // loop less propagation of blocks in network
            return;
        }
        if(rec_block->last_block !=-1) 
        if(!block_pointers[rec_block->last_block]){ // if we are getting a block whose parent haven't reached to us yet,
        //  we will push that block into temporary block pool
            if(block_is_in_tem_pool[rec_block]) return;
            block_is_in_tem_pool[rec_block] = true;
            temporary_block_pool.push_back(e);
            return;
        }
        struct block *t = last_block_in_longest_chain;
        if(validate_block(rec_block)){
            for(int i=0; i<peers_count; i++){
                if(peers[i] == e.from) continue;
                struct event t =e;
                t.event_t = block_recieved;
                t.from = id_m;
                t.intended_to = peers[i];
                double time = calculate_latency(peers[i],1 + rec_block->transactions.size());
                if(rec_block->last_block == -1) time = 0;
                event_queue.push({gv->current_time + time/1000.0, t});
            }
            if(!t) { create_next_block();} // if we just got a genesis block we will start working on next block
            else if(t != last_block_in_longest_chain){ create_next_block();} // if the block we just got is in the longest chain then also
            // we will start working on the next block
            event e = get_next_block_from_pool();
            if(e.mined_block){ // if there is a block in temporary pool and its parent is also present, then we will call recieve_block on that block 
                recieve_block(e);
            }
        }
        return;
    }

    void print_tree(){
            ofstream node_file;
            string filename = "node_" + to_string(id_m);
            node_file.open (filename);
            int n = gv->get_block_id();
            node_file << "digraph D { node [ordering=in]" << endl;
            for(int i=0; i<=n; i++){
                if(block_pointers[i]){
                    string miner = "miner_id__" + to_string(find_reciever_id(block_pointers[block_pointers[i]->block_id]->coinbase_transaction));
                    node_file << block_pointers[i]->block_id << "[label=" << miner << "]\n";
                }
            }
            for(int i=gv->node_count; i<=n; i++){
                if(block_pointers[i]){
                    node_file << block_pointers[i]->block_id << " -> " << block_pointers[i]->last_block<< endl;
                }
            }
            node_file << "}" << endl;
            node_file.close();
            return;
        }
        void write_data_to_file(){
            
            string path = "./blockchain_tree_data/node_" + to_string(id_m);
            ofstream node_file(path);
            if(link_speed_m == slow){
                node_file << "network speed = slow\n";
            }
            else {
                node_file << "network speed = fast\n";
            }
            if(computational_power_m == high){
                node_file << "computational power = high\n\n";
            }
            else {
                node_file << "computational power = low\n\n";
            }
            
            node_file << "Block data :\n";
            int n = gv->get_block_id();
            for(int i=0; i<=n; i++){
                if(block_pointers[i]){
                    node_file << " Block Id : " <<block_pointers[i]->block_id <<endl;
                    node_file << " Arrival time of block : " <<arrival_time_of_block[block_pointers[i]->block_id] <<endl;
                    node_file << " Parent of block : " <<block_pointers[i]->last_block <<endl << endl;
                }
            }
            node_file.close();
            return;
        }
};
