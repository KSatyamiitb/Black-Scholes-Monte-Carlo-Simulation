g++ -pthread base.cpp -o base

numSimulations=50
T=1
initPrice=100
impliedVol=20 # implied volatility in percentage
seed=123
for i in 100 1000 10000
do
    for j in 5 20 40 # 5% 20% 40% for N = 1000 case only, 20% for other cases
    do
        if [[ $i -eq 100  || $i -eq 10000 ]] && [[ $j -eq 5 || $j -eq 40 ]] 
        then
            seed=123
        else
            numTimePeriods=$i
            sigma=$j # realized volatility in percentage
            name=$numTimePeriods"_IV_"$impliedVol"_RealVol_"$sigma 
            echo $name
            ./base $numTimePeriods $numSimulations $T $initPrice $sigma $impliedVol $seed
            python3 plot.py $numTimePeriods $numSimulations $name
        fi
        
    done
done

