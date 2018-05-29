#!/bin/bash
declare -i amount_files=$1;
declare -i total=4*3*amount_files*2;
declare -i ctr=1;
for ((c=0; c<=3; c++)); do
    for ((r=1; r<=3; r++)); do
        for ((i=1; i<=amount_files; i++)); do
            for ((o=0; o<=1; o++)); do
                echo -ne "Export .smt $ctr/$total\r"
                if [ "$i" -lt 10 ]
                then
                    ./smt_export "-g" "input-files/game/game-00$i.txt" "-n" "input-files/navgraph/navgraph-costs-00$i.csv" "-r" "$r" "-c" "$c" "-o" "$o"
                elif [ "$i" -lt 100 ]
                then
                    ./smt_export "-g" "input-files/game/game-0$i.txt" "-n" "input-files/navgraph/navgraph-costs-0$i.csv" "-r" "$r" "-c" "$c" "-o" "$o"
                else
                    ./smt_export "-g" "input-files/game/game-$i.txt" "-n" "input-files/navgraph/navgraph-costs-$i.csv" "-r" "$r" "-c" "$c" "-o" "$o"
                fi
                ctr=$((ctr+1))
            done
        done
    done
done
