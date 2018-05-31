#!/bin/bash
declare timeout=60
declare -i amount_files=$1;
declare -i total=4*3*amount_files*2*3;
declare -i ctr=1;
for i in $(seq -f "%03g" 1 $amount_files); do
    for ((c=0; c<=1; c++)); do
        for ((r=1; r<=3; r++)); do
            for ((m=1; m<=1; m++)); do  # mode in which to run 1 -> normal, 2 -> macros, 3 -> deps
                for ((o=0; o<=1; o++)); do
                    /usr/bin/time -o "/tmp/runtime" -f "%e" timeout "$timeout" ./smt_export "-g" "input-files/game/game-$i.txt" "-n" "input-files/navgraph/navgraph-costs-$i.csv" "-r" "$r" "-c" "$c" "-o" "$o" "--check" "$m" &> /dev/null
                    if [ "$?" = "124" ]; then
                        echo -e "$i-C$c-R$r\\tM$m-o$o" \\t "__" \\t "${timeout}s"
                    else
            			echo -e "$i-C$c-R$r\\tM$m-o$o" \\t "OK" \\t "$(cat /tmp/runtime)s"
                    fi
                    ctr=$((ctr+1))
                done
            done
        done
    done
done
