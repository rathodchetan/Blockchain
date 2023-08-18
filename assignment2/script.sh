#!/bin/sh
g++ blockchain.cpp -o blockchain

for ((i=5; i<=70; i=i+5))
do
  ./blockchain 0 50 $i 50
done
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