rm -r experiments/*
./move_to_experiments.py $1
./parse_results.py
mv result_testing.xlsx excels/$1.xlsx

