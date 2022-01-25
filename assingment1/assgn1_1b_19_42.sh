#! /bin/bash
mkdir -p 1.b.files.out
for file in 1.b.files/*.txt
do
    cat $file | sort -n > "1.b.files.out/$(basename $file)";
done
sort -n 1.b.files.out/*.txt | uniq -c | awk '{print $2, $1}' > 1.b.out.txt
