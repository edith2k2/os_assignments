for i in ./files/*.txt ; do
  name=${i##*/}
  awk  '$1=$1{print NR,$0}' OFS="," $i > "./files_mod/$name"
done
