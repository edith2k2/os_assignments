for i in {1..150};do
  str=""
  for j in {1..10};do
    str+=$RANDOM","
  done
  echo $str >> file.csv
done
name=$1;col=$2;reg=$3;
str=$(awk -F ',' '$'$col' ~ /'$reg'/{print $'$col'}' $name )
if [[ ${#str} == 0 ]];then echo NO;
else echo YES; fi;
