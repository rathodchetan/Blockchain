#include <vector>
#include <string>

struct block{
    int block_id;
    int last_block;
    int timestamp;
    int chain_length;
    std::vector<std::string> transactions;
    std::string coinbase_transaction;
};