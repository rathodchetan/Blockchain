#!/bin/sh
g++ blockchain.cpp -o blockchain
./blockchain $1 $2
for dir in node_*
do
(
  dot -Tpng  $dir -o ./blockchain_trees/$dir.png
)
done
for dir in node_*
do
(
  rm $dir
)
done
exit 0