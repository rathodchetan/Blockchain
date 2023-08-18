#include<bits/stdc++.h>
#include "helper_function.cpp"
using namespace std;

class miner{

    protected:
        
        // transaction related variables
        vector<string> pending_transactions_in_longest_chain;
        vector<double> amount_of_all_nodes_in_longest_chain; 
        map<string,bool> transaction_already_listned;

        // block related variables
        map<int, struct block *> block_pointers;
        vector<event> temporary_block_pool;
        struct block *last_block_in_longest_chain;
        map<int, double> arrival_time_of_block;
        bool isSelfish;
        bool isStubborn;

    public:

        // properties of node
        network_speed link_speed_m;
        int id_m;
        cpu_power computational_power_m;

        // constructor
        miner (bool isSl, bool isSt, int id, cpu_power power, network_speed link_speed){
            id_m = id;
            computational_power_m = power;
            link_speed_m = link_speed;
            isSelfish = isSl;
            isStubborn = isSt;
            amount_of_all_nodes_in_longest_chain = vector<double>(gv->node_count,1000);
        }

        // to calculate latencies between two peers 
        double calculate_latency(int peer_id, double data_size){ // data size in kB
            double com_link_speed = gv->link_speed[id_m-1] and gv->link_speed[peer_id-1] ? 100.0 :5.0;  
            double queuing_delay = gv->link_speed[id_m-1] and gv->link_speed[peer_id-1] ? (*d_fast_lat)(*gen_fast_lat) : (*d_slow_lat)(*gen_fast_lat);
            double total_time = propagation_delay[{id_m,peer_id}] + data_size/com_link_speed + queuing_delay;
            // if(isStubborn) return 0;
            return total_time;
        }
        void create_genesis_block(){
            // genesis block
            struct block * gen_block = new struct block();
            gen_block->block_id = gv->get_block_id();
            gen_block->last_block=-1;
            gen_block->chain_length =1;
            gen_block->coinbase_transaction = to_string(gv->get_transaction_id()) +  ":" + to_string(id_m) +" mines 50 coins";
            
            // event for block mining
            double mining_time = gv->current_time + (computational_power_m == high ? (*d_fast_mining)(*gen_fast_mining) : (*d_slow_mining)(*gen_fast_mining));
            gen_block->timestamp = mining_time;
            struct event t;
            t.event_t = block_mined;
            t.from = id_m;
            t.mined_block = gen_block;
            gen_block->timestamp = mining_time;
            event_queue.push({mining_time, t});
            return;
        }
        // whenever we are adding a block in longest chain or adding that block into the chain is making that chain longest
        // we are calling this function
        // it will create next block and will choose a mining time from exponential distribution
        void create_next_block(){
            struct block * next_block = new struct block();
            next_block->block_id = gv->get_block_id();
            next_block->last_block = last_block_in_longest_chain->block_id;
            next_block->chain_length = last_block_in_longest_chain->chain_length+1;
            next_block->coinbase_transaction = to_string(gv->get_transaction_id()) +  ":" + to_string(id_m) +" mines 50 coins";

            int pen_tr = pending_transactions_in_longest_chain.size();
            vector<double> am = amount_of_all_nodes_in_longest_chain;
            int transactions_included =0;
            int i=0;
            // adding transaction into the block and verifying simultaneously

            for(; i<pen_tr; i++){
                double amount = find_amount(pending_transactions_in_longest_chain[i]);
                int sender = find_sender_id(pending_transactions_in_longest_chain[i])-1;
                int reciver = find_reciever_id(pending_transactions_in_longest_chain[i])-1;
                if(am[sender] < amount + 1 ){
                    continue;
                }
                transactions_included++;
                am[sender] -= amount;

                next_block->transactions.push_back(pending_transactions_in_longest_chain[i]);
                if(transactions_included == 998) break;
            }
            vector<string> tem_tran;
            i++;
            for(; i< pen_tr; i++){
                tem_tran.push_back(pending_transactions_in_longest_chain[i]);
            }
            pending_transactions_in_longest_chain = tem_tran;
            // tem_tran.clear();
            
            // event for block mining
            double mining_time = gv->current_time + (computational_power_m == high ? (*d_fast_mining)(*gen_fast_mining) : (*d_slow_mining)(*gen_fast_mining));
            if(isSelfish or isStubborn) mining_time =  gv->current_time + (*d_adversary_mining)(*gen_adversary_mining);
            struct event t;
            t.event_t = block_mined;
            t.from = id_m;
            t.mined_block = next_block;
            next_block->timestamp = mining_time;
            event_queue.push({mining_time, t}); // pushing the event into the queue
            return;
        }

        // function to validate genesis block 
        bool validate_genesis_block(struct block *bl){
            if(bl->transactions.size() > 0) return false;
            if(find_amount(bl->coinbase_transaction) >50) return false;
            if(bl->chain_length !=1) return false;
            last_block_in_longest_chain = bl;
            block_pointers[bl->block_id] = bl;
            arrival_time_of_block[bl->block_id] = gv->current_time;
            return true;
        }

        // function to validate any block in longest chain other than genesis block
        bool validate_block_in_longest_chain(struct block *bl){
            if(find_amount(bl->coinbase_transaction) >50) return false;
            if(bl->chain_length != block_pointers[bl->last_block]->chain_length+1) return false;
            vector<double> amounts = amount_of_all_nodes_in_longest_chain;
            for(auto tran : bl->transactions){ // temporary checking of amounts 
                double amount = find_amount(tran);
                int sender = find_sender_id(tran);
                amounts[sender-1] -= amount;
                if(amounts[sender-1] <0) return false;
            }
            amounts.clear();
            for(auto tran : bl->transactions){ // making permanenent changes into amounts
                double amount = find_amount(tran);
                int sender = find_sender_id(tran);
                int reciver = find_reciever_id(tran);
                amount_of_all_nodes_in_longest_chain[sender-1] -= amount;
                amount_of_all_nodes_in_longest_chain[reciver-1] += amount;
            }
            int coinbase_rec = find_reciever_id(bl->coinbase_transaction);
            amount_of_all_nodes_in_longest_chain[coinbase_rec-1] +=50;
            // removing transactions from pending transactions
            for(auto trans : bl->transactions){
                transaction_already_listned[trans] = true;
            }
            map<string, bool> m;
            for(auto tran : bl->transactions){
                m[tran] = true;
            }
            vector<string> temp;
            for(auto tran : pending_transactions_in_longest_chain){
                if(!m[tran]) temp.push_back(tran);
            }
            pending_transactions_in_longest_chain = temp;
            block_pointers[bl->block_id] = bl;
            last_block_in_longest_chain = bl; // setting new last block in chain
            arrival_time_of_block[bl->block_id] = gv->current_time;
            return true;
        }

        // finding closest comman parent for last block of given block and last block in longest chain 
        int closest_common_parent(struct block *bl){
            int bl_no=last_block_in_longest_chain->block_id;
            while(block_pointers[bl_no]->chain_length != block_pointers[bl->last_block]->chain_length){
                bl_no = block_pointers[bl_no]->last_block;
            }
            int bl_no_or = block_pointers[bl->last_block]->block_id;
            while(bl_no != bl_no_or){
                bl_no = block_pointers[bl_no]->last_block;
                bl_no_or = block_pointers[bl_no_or]->last_block;
            }
            return bl_no;
        }

        // finding pending transactions and amounts in the last block of the given block in the orphaned chain
        vector<string> find_correct_tran_and_amount(int bl_no, vector<string> &pen_tras, vector<double> &am, struct block *bl){
            int t_bl = last_block_in_longest_chain->block_id;
            vector<string> ret;
            while(t_bl != bl_no){
                for(auto tr : block_pointers[t_bl]->transactions){
                    double amount = find_amount(tr);
                    int sender = find_sender_id(tr);
                    int reciver = find_reciever_id(tr);
                    am[sender-1] += amount;
                    am[reciver-1] -= amount;
                    pen_tras.push_back(tr);
                }
                int coinbase_rec = find_reciever_id(block_pointers[t_bl]->coinbase_transaction);
                am[coinbase_rec-1] -=50;
                t_bl = block_pointers[t_bl]->last_block;
            }
            t_bl = block_pointers[bl->last_block]->block_id;
            map<string,bool> pr;
            while(t_bl != bl_no){
                for(auto tr : block_pointers[t_bl]->transactions){
                    double amount = find_amount(tr);
                    int sender = find_sender_id(tr);
                    int reciver = find_reciever_id(tr);
                    am[sender-1] -= amount;
                    am[reciver-1] += amount;
                    pr[tr] = true;
                }
                int coinbase_rec = find_reciever_id(block_pointers[t_bl]->coinbase_transaction);
                am[coinbase_rec-1] +=50;
                t_bl = block_pointers[t_bl]->last_block;
            }
            for( auto tr: pen_tras){
                if(!pr[tr]){
                    ret.push_back(tr);
                }
            }
            return ret;

        }

        // function to validate block in orphaned chain
        bool validate_block_in_orphaned_chain(struct block *bl){
            int bl_no = closest_common_parent(bl);
            vector<string> pen_tras = pending_transactions_in_longest_chain;
            vector<double> am = amount_of_all_nodes_in_longest_chain;
            vector<string> p_tr = find_correct_tran_and_amount(bl_no, pen_tras, am ,bl);

            if(find_amount(bl->coinbase_transaction) >50) return false;
            if(bl->chain_length != block_pointers[bl->last_block]->chain_length+1) return false;
            vector<double> amounts = am;
            // temporary check of amounts 
            for(auto tran : bl->transactions){
                double amount = find_amount(tran);
                int sender = find_sender_id(tran);
                amounts[sender-1] -= amount;
                if(amounts[sender-1] <0) return false;
            }
            // if everything is good then permanently change balances
            for(auto tran : bl->transactions){
                double amount = find_amount(tran);
                int sender = find_sender_id(tran);
                int reciver = find_reciever_id(tran);
                am[sender-1] -= amount;
                am[reciver-1] += amount;
            }
            int coinbase_rec = find_reciever_id(bl->coinbase_transaction);
            am[coinbase_rec-1] +=50;
                    map<string, bool> m;
            
            // remove pending transactions which are in the block
            for(auto tran : bl->transactions){
                m[tran] = true;
            }
            vector<string> temp;
            for(auto tran : p_tr){
                if(!m[tran]) temp.push_back(tran);
            }
            p_tr = temp;
            block_pointers[bl->block_id] = bl;

            // if this chain has become longest after adding the current block then make this chain longest and set some variables
            if(bl->chain_length > last_block_in_longest_chain->chain_length){
                for(auto trans : bl->transactions){
                    transaction_already_listned[trans] = true;
                }
                pending_transactions_in_longest_chain = p_tr;
                amount_of_all_nodes_in_longest_chain = am;
                last_block_in_longest_chain = bl;
            }
            arrival_time_of_block[bl->block_id] = gv->current_time;
            return true;
            
        }

        // general function to validate all blocks
        bool validate_block(struct block *bl){
            if(bl->last_block == -1){
                return validate_genesis_block(bl);
            }
            if(block_pointers[bl->last_block] == last_block_in_longest_chain){
                return validate_block_in_longest_chain(bl);
            }
            return validate_block_in_orphaned_chain(bl);
        }


        event get_next_block_from_pool(){
            int c = -1;
            struct event e;
            e.mined_block = NULL;
            for(auto i: temporary_block_pool){
                if(block_pointers[i.mined_block->last_block]){
                    c = i.mined_block->block_id;
                    e = i;
                    break;
                }
            }
            if(c==-1) return e;
            vector<event> tem;
            for(auto i: temporary_block_pool){
                if(i.mined_block->block_id == c) continue;
                tem.push_back(i);
            }
            temporary_block_pool = tem;
            return e;
        }
        
        struct block * get_last_block_in_longest_chain(){
            return last_block_in_longest_chain;
        }
        
};