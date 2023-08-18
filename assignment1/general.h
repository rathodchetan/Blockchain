
class general_global_variables {
        int transaction_id;
        int block_id;
    
    public:
        // general simulator related variables
        double current_time;
        int node_count;
        int high_cpu_nodes;
        int low_cpu_nodes;
        network_speed *link_speed;

        // parameters for exponential distributions to get mining time for a block
        double mean_low_cpu_hashing_time;
        double mean_high_cpu_hashing_time;

        // tunable parameters for simulator
        double mean_transaction_time;
        double mean_block_interarrival_time;

    general_global_variables(int n, double m_t, double m_b){
        node_count = n;
        transaction_id =0;
        block_id =0;
        current_time=0;
        high_cpu_nodes=0;
        low_cpu_nodes=0;
        mean_transaction_time = m_t;
        mean_block_interarrival_time = m_b;
    }

    int get_transaction_id(){
        transaction_id++;
        return transaction_id-1;
    }
    int get_block_id(){
        block_id++;
        return block_id-1;
    }

};